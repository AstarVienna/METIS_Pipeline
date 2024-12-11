import cpl

from cpl.core import Msg

from pymetis.inputs.base import PipelineInput


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: [str] = None,
                 required: bool = None,
                 **kwargs):
        self.frame: cpl.ui.Frame | None = None
        super().__init__(tags=tags, required=required, **kwargs)

        self.tag_match = {}
        for frame in frameset:
            if match := self.tags.fullmatch(frame.tag):
                if self.frame is None:
                    Msg.debug(self.__class__.__qualname__,
                              f"Found a {self.title} frame: {frame.file}.")
                else:
                    Msg.warning(self.__class__.__qualname__,
                                f"Found another {self.title} frame: {frame.file}! "
                                f"Discarding previously loaded {self.frame.file}.")
                self.frame = frame
                self.tag_match = match.groupdict()
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Ignoring {frame.file}: tag {frame.tag} does not match.")

        self.extract_tag_parameters()

    def extract_tag_parameters(self):
        if self.tag_match is not None:
            for key in self.tag_match.keys():
                Msg.debug(self.__class__.__qualname__,
                          f"Matched a tag parameter: '{key}' = '{self.tag_match[key]}'.")

        self._detector = self.tag_match.get('detector', None)

    def verify(self):
        """
        Run all the required instantiation time checks
        """
        self._verify_frame_present(self.frame)

    def _verify_frame_present(self,
                              frame: cpl.ui.Frame) -> None:
        """
        Verification shorthand: if a required frame is not present, i.e. `None`,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        If it is not required, emit a warning but continue.
        """
        if frame is None:
            if self.required:
                raise cpl.core.DataNotFoundError(f"No {self.title} frame found in the frameset.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"No {self.title} frame found, but not required.")
        else:
            Msg.debug(self.__class__.__qualname__,
                      f"Found a {self.title} frame {frame.file}")
