import cpl

from prototypes.base.input import RecipeInput


class GainMapInputMixin(RecipeInput):
    tags_gain_map = [] # ["GAIN_MAP_det"]

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.gain_map: cpl.core.Image | None = None

        if not self.tags_gain_map:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_gain_map`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "gain map"

        if frame.tag in self.tags_gain_map:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.gain_map, frame, origin=self.__class__.__qualname__, title=title)
            self.gain_map = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.gain_map, "gain map")
        super().verify()


class GainMap2rgInputMixin(GainMapInputMixin):
    """ A gain map for the IFU """
    tags_gain_map = ["GAIN_MAP_2RG"]


class GainMapGeoInputMixin(GainMapInputMixin):
    """ A gain map for the N band """
    tags_gain_map = ["GAIN_MAP_GEO"]