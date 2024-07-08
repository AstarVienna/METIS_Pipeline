from abc import ABCMeta, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg


class MetisRecipeImpl(metaclass=ABCMeta):
    """ A base class for METIS recipe implementations """
    parameters = cpl.ui.ParameterList([])

    # Available parameters are a class variable
    parameters = cpl.ui.ParameterList([])

    def __init__(self, recipe) -> None:
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.frameset = None
        self.header = None
        self.raw_frames = cpl.ui.FrameSet()
        self.raw_images = cpl.core.ImageList()
        self.product_frames = cpl.ui.FrameSet()

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """ Main function of the recipe, must have this signature """

        self.frameset = frameset            # First save the frameset

        self.import_settings(settings)      # Import and process the provided settings dict
        self.load_input_frameset(frameset)  # Load the input raw frames
        self.verify_input()                 # Verify that it is valid (maybe with `schema` too?)
        self.categorize_raw_frames()        # Categorize raw images based on keywords
        self.process_images()               # Do the actual processing
        self.save_product()                 # Save the output product

        return self.product_frames

    def import_settings(self, settings: Dict[str, Any]) -> None:
        """ Update the recipe parameters with the values requested by the user """
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(self.name,
                            f"Settings includes '{key}':{value} "
                            f"but class {self.__class__.__name__} "
                            f"has no parameter named {key}.")

    def categorize_raw_frames(self) -> None:
        """ Filter raw frames from the SOF """

        for idx, frame in enumerate(self.raw_frames):
            Msg.info(self.name, f"Processing raw frame #{idx}: {frame.file!r}...")

            Msg.debug(self.name, f"Loading image {frame.file}")
            header = cpl.core.PropertyList.load(frame.file, 0)
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            self.raw_images.insert(idx, raw_image)

    @abstractmethod
    def load_input_frameset(self, frameset) -> cpl.ui.FrameSet:
        """ Load and categorize the frameset. """
        return cpl.ui.FrameSet()

    @abstractmethod
    def verify_input(self) -> None:
        """
            Verify that the loaded frameset is valid and conforms to the specification.
            It would be also good to do this with some schema.
            Returns None if OK, otherwise an exception is raised.

            For demonstration purposes we raise an exception here. Real world
            recipes should rather print a message (also to have it in the log file)
            and exit gracefully.
            [Martin]: Shouldn't this be esorex's problem?
        """
        return None

    @abstractmethod
    def process_images(self) -> cpl.ui.FrameSet:
        return cpl.ui.FrameSet()

    def save_product(self) -> cpl.ui.FrameSet:
        """ Register the created product """
        for name, product in self.products.items():
            product.save()
            self.product_frames.append(product.create_frame())

        return self.product_frames

    @property
    @abstractmethod
    def detector_name(self) -> str:
        return "<invalid>"
