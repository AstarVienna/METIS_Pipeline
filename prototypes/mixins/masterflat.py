import cpl

from prototypes.input import PipelineInput


class MasterFlatInputMixin(PipelineInput):
    tags_flat: [str] = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.master_flat: cpl.core.Image | None = None

        if not self.tags_flat:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_flat`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "master flat"
        match frame.tag:
            case tag if tag in self.tags_flat:
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self._override_with_warning(self.master_flat, frame, origin=self.__class__.__qualname__, title=title)
                self.master_flat = frame
            case _:
                super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.master_flat, "master flat")
        super().verify()
