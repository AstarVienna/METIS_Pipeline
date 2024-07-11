import cpl

from cpl.core import Msg

from .base import MetisRecipeImpl


class RawImageProcessor(MetisRecipeImpl):
    def __init__(self, recipe):
        super().__init__(recipe)
        self.raw_frames = cpl.ui.FrameSet()
        self.raw_images = cpl.core.ImageList()

    def verify_input_frames(self) -> None:
        """ RawImageProcessor mixin wants to see a bunch of raw frames. """
        if len(self.raw_frames) == 0:
            raise cpl.core.DataNotFoundError("No raw frames found in the frameset.")

    def load_input_images(self) -> cpl.core.ImageList:
        """ Always load a set of raw images """
        for idx, frame in enumerate(self.raw_frames):
            Msg.info(self.name, f"Processing input frame #{idx}: {frame.file!r}...")

            # Append the loaded image to an image list
            Msg.debug(self.name, f"Loading input image {frame.file}")
            self.raw_images.append(cpl.core.Image.load(frame.file, extension=1))

        return self.raw_images
