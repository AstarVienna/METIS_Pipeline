import cpl
from cpl.core import Msg

from prototypes.rawimage import RawImageProcessor


class FlatInput(RawImageProcessor.Input):
    """
    Base class for Inputs which create flats. Requires a bunch of raw frames and a master dark.
    """
    tag_raw = None
    tags_dark = None

    def __init__(self, frameset: cpl.ui.FrameSet):
        self.master_dark: cpl.ui.Frame | None = None
        super().__init__(frameset)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        match frame.tag:
            case self.tag_raw:
                frame.group = cpl.ui.Frame.FrameGroup.RAW
                self.raw.append(frame)
                Msg.debug(self.__class__.__qualname__, f"Got raw frame: {frame.file}.")
            case tag if tag in self.tags_dark:
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.master_dark = frame
                Msg.debug(self.__class__.__qualname__, f"Got master dark frame: {frame.file}.")
            case _:
                super().categorize_frame(frame)

    def verify(self) -> None:
        # First verify the raw frames (provided by base class)
        super().verify()

        print(self.master_dark)
        if self.master_dark is None:
            raise cpl.core.DataNotFoundError("No masterdark frames found in the frameset.")
