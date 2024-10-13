from abc import abstractmethod
from xml.sax.handler import property_dom_node

import cpl

from cpl.core import Msg


class PipelineInput:
    _title: str = None                      # No univerrsal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _tags: [str] = None                     # No universal tags are provided
    _group: str = None

    @property
    def title(self):
        return self._title

    @property
    def tags(self):
        return self._tags

    @property
    def required(self):
        return self._required

    @property
    def group(self):
        return self._group

    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 **kwargs):
        if self.title is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        if tags is not None:
            self._tags = tags

        # Now expand the tags with local context
        try:
            print(self._tags, kwargs)
            self._tags = [tag.format(**kwargs) for tag in self._tags]
        except KeyError as e:
            Msg.error(self.__class__.__qualname__, f"Could not substitute tag placeholders: {e}")
            raise e

        # Check if tags are defined...
        if not self.tags:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tags")

        # ...and that they are a list of strings (not a single string!)
        if not isinstance(self.tags, list):
            raise TypeError(f"Tags must be a list of template strings, got '{self.tags}'")

        # Check is frame_group is defined (if not, this gives rise to very strange bugs deep within CPL)
        if not self.group:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined group")

    @abstractmethod
    def verify(self) -> None:
        """
        Verify that the input has all the required inputs
        """

    def print_debug(self, *, offset: int = 0):
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}Tags: {self.tags}")


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present. Also provides methods for basic validation.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 **kwargs):
        self.frame: cpl.ui.Frame | None = None
        super().__init__(frameset, tags=tags, **kwargs)

        for frame in frameset:
            if frame.tag in self.tags:
                if self.frame is None:
                    Msg.debug(self.__class__.__qualname__,
                              f"Found a {self.title} frame: {frame.file}.")
                else:
                    Msg.warning(self.__class__.__qualname__,
                                f"Found another {self.title} frame: {frame.file}! "
                                f"Discarding previously loaded {self.frame.file}.")
                self.frame = frame

    def verify(self):
        self._verify_frame_present(self.frame)

    def _verify_frame_present(self,
                              frame: cpl.ui.Frame) -> None:
        """
        Verification shorthand: if a required frame is not present, i.e. `None`,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if frame is None:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} found in the frameset.")
            else:
                Msg.debug(f"{self.title} not found but not required.")
        else:
            Msg.debug(self.__class__.__qualname__,
                      f"Found a {self.title} frame {frame.file}")


class MultiplePipelineInput(PipelineInput):
    """
    A pipeline input that expects multiple frames, such as raw.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 **kwargs):                     # Any other args
        self.frameset: cpl.ui.FrameSet | None = cpl.ui.FrameSet()
        super().__init__(frameset, tags=tags, **kwargs)

        for frame in frameset:
            if frame.tag in self.tags:
                frame.group = self.group
                self.frameset.append(frame)
                Msg.debug(self.__class__.__qualname__,
                          f"Found a {self.title} frame: {frame.file}.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Ignoring {frame.file}: tag {frame.tag} not in {self.tags}.")


    def verify(self):
        self._verify_frameset_not_empty()
        self._verify_same_detector()

    def _verify_frameset_not_empty(self) -> None:
        """
        Verification shorthand: if a required frameset is not present or empty,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if (count := len(self.frameset)) == 0:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} found in the frameset.")
            else:
                Msg.debug(f"{self.title} not found but not required.")
        else:
            Msg.debug(self.__class__.__qualname__, f"OK: {count} frames found")

    def _verify_same_detector(self) -> None:
        """
        Verify whether all the raw frames originate from the same detector.

        Raises
        ------
        KeyError
            If the found detector name is not a valid detector name
        ValueError
            If dark frames from more than one detector are found

        Returns
        -------
        None:
            None on success
        """
        detectors = []

        for frame in self.frameset:
            header = cpl.core.PropertyList.load(frame.file, 0)
            det = header['ESO DPR TECH'].value
            try:
                detectors.append({
                                     'IMAGE,LM': '2RG',
                                     'IMAGE,N': 'GEO',
                                     'IFU': 'IFU',
                                 }[det])
            except KeyError as e:
                raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e

        # Check if all the raws have the same detector, if not, we have a problem
        if len(unique := list(set(detectors))) == 1:
            self._detector_name = unique[0]
        else:
            raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")
