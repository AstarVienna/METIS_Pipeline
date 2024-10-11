from abc import abstractmethod
from xml.sax.handler import property_dom_node

import cpl

from cpl.core import Msg


class PipelineInput:
    _title: str = None                      # No univerrsal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _tags: [str] = None                     # No universal tags are provided
    _tag_kwargs: dict[str, str] = None
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
                 **kwargs):
        if self.title is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        self._tags = [tag.format(**kwargs) for tag in self._tags]

        if not self.tags:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tags")

        if not self.group:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined group")

    @abstractmethod
    def verify(self) -> None:
        """
        Verify that the input has all the required inputs
        """


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 **kwargs):
        self.frame: cpl.ui.Frame | None = None
        super().__init__(frameset, **kwargs)

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
                 **kwargs):                     # Any other args
        self.frameset: cpl.ui.FrameSet | None = cpl.ui.FrameSet()
        super().__init__(frameset, **kwargs)

        for frame in frameset:
            if frame.tag in self.tags:
                frame.group = self.group
                self.frameset.append(frame)
                Msg.debug(self.__class__.__qualname__,
                          f"Found a {self.title} frame: {frame.file}.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Ignoring {frame.file}, {self.tags}.")


    def verify(self):
        self._verify_frameset_not_empty(self.frameset)

    def _verify_frameset_not_empty(self, frameset: cpl.ui.FrameSet) -> None:
        """
        Verification shorthand: if a required frameset is not present or empty,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        """
        if len(self.frameset) == 0:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} found in the frameset.")
            else:
                Msg.debug(f"{self.title} not found but not required.")
