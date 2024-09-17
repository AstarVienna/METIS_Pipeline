import cpl
from cpl.core import Msg

from prototypes.input import PipelineInput


class PersistenceMixin(PipelineInput):
    tags_persistence = ["PERSISTENCE_MAP"]

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.persistence_map: cpl.core.Image | None = None

        super().__init__(frameset, **kwargs)

    def verify(self) -> None:
        self._verify_frame_present(self.persistence_map, "persistence_map")
        super().verify()

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "persistence map"
        match frame.tag:
            case tag if tag in self.tags_persistence:
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                if self.persistence_map is None:
                    Msg.debug(self.__class__.__qualname__,
                              f"Got {title} frame: {frame.file}.")
                else:
                    Msg.warning(self.__class__.__qualname__,
                                f"Got another {title} frame: {frame.file}. "
                                f"Discarding previously loaded {self.persistence_map.file}.")
                    self.persistence_map = frame
            case _:
                super().categorize_frame(frame)
