from abc import ABCMeta, abstractmethod
from typing import Dict, Any
from schema import Schema

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

    class Input(metaclass=ABCMeta):
        def __init__(self, frameset: cpl.ui.FrameSet):
            """ Filter the input frameset, capture frames that match criteria and assign them to own attributes. """

            for frame in frameset:
                self.categorize_frame(frame)

        @abstractmethod
        def categorize_frame(self, frame):
            """ Inspect a single frame and assign it to the proper attribute. """
            # If we got all the way up here, no one recognizes this frame, warn!
            Msg.warning(self.__class__.__name__,
                        f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

        @abstractmethod
        def verify(self):
            """ Verify that the input frameset conforms to requirements. """

    def __init__(self, recipe: cpl.ui.PyRecipe) -> None:
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.input = None
        self.frameset = None
        self.header = None
        self.product_frames = cpl.ui.FrameSet()
        self.products = {}

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """ Main function of the recipe implementation """

        try:
            self.frameset = frameset
            self.import_settings(settings)      # Import and process the provided settings dict

            self.input = self.Input(frameset)   # Create an appropriate Input object
            self.validate_input()
            self.verify_input_frames()          # Verify that they are valid (maybe with `schema` too?)
            self.process_images()               # Do the actual processing
            self.save_products()                # Save the output products
        except cpl.core.DataNotFoundError as e:
            Msg.warning(f"Data not found: {e.message}")

        return self.get_product_frameset()      # Return the output as a pycpl FrameSet

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

    @abstractmethod
    def verify_input_frames(self) -> None:
        """
            Verify that the loaded frameset is valid and conforms to the specification.
            It would be also good to do this with some schema (but that might make Lars unhappy).
            Returns None if OK, otherwise an exception is raised.

            For demonstration purposes we raise an exception here. Real world
            recipes should rather print a message (also to have it in the log file)
            and exit gracefully.
            [Martin]: Shouldn't this be esorex's problem? I want exceptions!
        """

    @abstractmethod
    def load_input_images(self) -> None:
        """ Load the filtered frames from the frameset """

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


class MetisRecipe(cpl.ui.PyRecipe):
    _name = "Base METIS recipe"
    _version = "0.0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "CPL-3.0-or-later"
    _synopsis = "Abstract-like base class for METIS recipes"
    _description = ("This class serves as the base class for all METIS recipes. "
                    "In an ideal world it would also be abstract, but then pyseorex would instantiate it and crash.")

    parameters = cpl.ui.ParameterList([])
    implementation_class = str      # Dummy class, this has to be instantiated but not used

    def __init__(self):
        super().__init__()
        self.implementation = self.implementation_class(self)

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        return self.implementation.run(frameset, settings)


