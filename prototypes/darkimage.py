from abc import ABC

import cpl
from cpl.core import Msg

from prototypes.rawimage import RawImageProcessor


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    DarkImageProcessor is a subclass of RawImageProcessor that also requires a single `master_dark` frame.
    Provides methods for loading and verification of the dark frame, warns if multiple master darks are provided, etc.
    """
    class Input(RawImageProcessor.Input):
        tags_dark: [str] = []

        def __init__(self, frameset: cpl.ui.FrameSet) -> None:
            self.master_dark: cpl.ui.Frame | None = None
            if not self.tags_dark:
                raise NotImplementedError("DarkImageProcessor Input must define `tags_dark`")
            super().__init__(frameset)

        def load_dark_frame(self) -> cpl.core.Image:
            return cpl.core.Image(self.master_dark)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case tag if tag in self.tags_dark:
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    if self.master_dark is None:
                        Msg.debug(self.__class__.__qualname__,
                                  f"Got raw frame: {frame.file}.")
                    else:
                        Msg.warning(self.__class__.__qualname__,
                                    f"Got another dark frame: {frame.file}. "
                                    f"Discarding previously loaded {self.master_dark.file}.")
                    self.master_dark = frame
                case _:
                    super().categorize_frame(frame)

        def verify(self) -> None:
            if self.master_dark is None:
                raise cpl.core.DataNotFoundError("No master dark frame found in the frameset.")
            super().verify()
