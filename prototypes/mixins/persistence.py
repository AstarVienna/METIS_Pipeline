import cpl

from prototypes.input import PipelineInput


class PersistenceInputMixin(PipelineInput):
    tags_persistence = ["PERSISTENCE_MAP"]

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.persistence_map: cpl.core.Image | None = None

        if not self.tags_persistence:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_persistence`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "persistence map"

        if frame.tag in self.tags_persistence:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.persistence_map, frame, origin=self.__class__.__qualname__, title=title)
            self.persistence_map = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.persistence_map, "persistence map")
        super().verify()
