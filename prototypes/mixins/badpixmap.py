import cpl

from prototypes.base.input import RecipeInput


class BadpixMapInputMixin(RecipeInput):
    tags_badpix_map: [str] = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.badpix_map: cpl.core.Image | None = None

        if not self.tags_badpix_map:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_badpixmap`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "badpix map"

        if frame.tag in self.tags_badpix_map:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.badpix_map, frame, origin=self.__class__.__qualname__, title=title)
            self.badpix_map = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.badpix_map, "badpix map")
        super().verify()
