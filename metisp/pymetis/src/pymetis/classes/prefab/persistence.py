from cpl.core import ImageList, Msg


class PersistenceCorrectionMixin:
    """
    A mixin that performs persistence correction.
    """
    def correct_persistence(self, raw_images: ImageList) -> ImageList:
        """
        Correct the raw image list for persistence.

        # FixMe Currently only a mockup, does not actually do anything.
        """
        persistence = self.inputset.persistence_map.load_data(extension=rf'PERSISTENCE_MAP')
        raw_images.subtract_image(persistence)

        Msg.info(self.__class__.__qualname__, f"Pretending to do persistence correction")
        return raw_images
