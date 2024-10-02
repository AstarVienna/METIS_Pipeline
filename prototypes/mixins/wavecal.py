import cpl

from prototypes.input import PipelineInput


class WavecalInputMixin(PipelineInput):
    tags_wavecal = ["IFU_WAVECAL"]

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.wavecal: cpl.core.Image | None = None

        if not self.tags_wavecal:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_wavecal`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "wavelength calibration image"

        if frame.tag in self.tags_wavecal:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.wavecal, frame, origin=self.__class__.__qualname__, title=title)
            self.wavecal = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.wavecal, "wavecal")
        super().verify()
