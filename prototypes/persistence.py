import cpl


class PersistenceMixin:
    tags_persistence = ["PERSISTENCE_MAP"]

    def __init__(self, **kwargs):
        self.persistence_map: cpl.core.Image | None = None

    def verify(self) -> None:
        if self.persistence_map is None:
            raise cpl.core.DataNotFoundError("No persistence map found in the dataset.")