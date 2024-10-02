import cpl

from prototypes.input import PipelineInput
from prototypes.mixins.persistence import PersistenceInputMixin
from prototypes.mixins.linearity import LinearityInputMixin
from prototypes.mixins.badpixmap import BadpixMapInputMixin


class MasterDarkInputMixin(PipelineInput):
    tags_dark: [str] = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.master_dark: cpl.core.Image | None = None

        if not self.tags_dark:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_dark`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "master dark"

        if frame.tag in self.tags_dark:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.master_dark, frame, origin=self.__class__.__qualname__, title=title)
            self.master_dark = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.master_dark, "master dark")
        super().verify()
