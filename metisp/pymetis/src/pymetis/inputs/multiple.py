from typing import Pattern

import cpl

from cpl.core import Msg

from pymetis.inputs.base import PipelineInput


class MultiplePipelineInput(PipelineInput):
    """
    A pipeline input that expects multiple frames, such as raw processor.
    """
    def __init__(self,
                 frameset: cpl.ui.FrameSet,
                 *,
                 tags: Pattern = None,
                 required: bool = None,
                 **kwargs):                     # Any other args
        self.frameset: cpl.ui.FrameSet | None = cpl.ui.FrameSet()
        super().__init__(tags=tags, required=required, **kwargs)

        tag_matches = []
        for frame in frameset:
            if match := self.tags.fullmatch(frame.tag):
                frame.group = self.group
                self.frameset.append(frame)
                Msg.debug(self.__class__.__qualname__,
                          f"Matched a {self.title} frame '{frame.file}' (with {match.groupdict()}).")
                tag_matches.append(match.groupdict())
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Ignoring {frame.file}: tag {frame.tag} does not match.")

        self.extract_tag_parameters(tag_matches)

    def extract_tag_parameters(self, matches: [dict[str, str]]):
        params = {}

        for match in matches:
            params |= match

        self.tag_parameters = {}

        self._detector = params.get('detector', None)

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
                raise cpl.core.DataNotFoundError(f"No {self.title} frames found in the frameset.")
            else:
                Msg.debug(self.__class__.__qualname__, f"No {self.title} frames found but not required.")
        else:
            Msg.debug(self.__class__.__qualname__, f"Frameset OK: {count} frame{'s' if count > 1 else ''} found")

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
