from abc import ABCMeta, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.product import PipelineProduct


class MetisRecipeImpl(metaclass=ABCMeta):
    """
        An abstract base class for all METIS recipe implementations
        Contains main flow control and provides abstract methods to be overridden
        by particular pipeline recipe implementations.
    """
    # Available parameters are a class variable. This must be present, even if empty.
    parameters = cpl.ui.ParameterList([])

    def __init__(self, recipe: cpl.ui.PyRecipe) -> None:
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.frameset = None
        self.header = None
        self.input_frames = cpl.ui.FrameSet()
        self.input_images = cpl.core.ImageList()
        self.product_frames = cpl.ui.FrameSet()
        self.products = {}

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """ Main function of the recipe implementation """

        self.frameset = frameset
        self.import_settings(settings)      # Import and process the provided settings dict
        self.categorize_frameset()          # Categorize raw frames based on keywords
        self.load_input_frames()            # Load the actual input raw frames
        self.verify_input()                 # Verify that they are valid (maybe with `schema` too?)
        self.process_images()               # Do the actual processing
        self.save_products()                # Save the output products
        return self.get_product_frameset()  # Return the output as a pycpl FrameSet

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

    def load_input_frames(self) -> cpl.core.ImageList:
        """ Load and the filtered frames from the frameset """

        for idx, frame in enumerate(self.input_frames):
            Msg.info(self.name, f"Processing input frame #{idx}: {frame.file!r}...")
            header = cpl.core.PropertyList.load(frame.file, 0)

            # Append the loaded image to an image list
            Msg.debug(self.name, f"Loading input image {frame.file}")
            self.input_images.append(cpl.core.Image.load(frame.file, extension=1))

        return self.input_images

    @abstractmethod
    def categorize_frameset(self) -> None:
        """ Filter raw frames from the SOF """

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

    @abstractmethod
    def process_images(self) -> Dict[str, PipelineProduct]:
        return {}

    def save_products(self) -> None:
        """ Register the created product """
        for name, product in self.products.items():
            Msg.debug(__name__, f"Saving {product}")
            product.save()

    def get_product_frameset(self) -> cpl.ui.FrameSet:
        product_frames = cpl.ui.FrameSet()

        for name, product in self.products.items():
            product_frames.append(product.as_frame())

        return product_frames

    @property
    @abstractmethod
    def detector_name(self) -> str:
        return "<invalid>"
