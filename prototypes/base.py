from abc import ABC, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.product import PipelineProduct
from prototypes.input import PipelineInput


class MetisRecipeImpl(ABC):
    """
        An abstract base class for all METIS recipe implementations.
        Contains central flow control and provides abstract methods to be overridden
        by particular pipeline recipe implementations.
    """
    Input = PipelineInput
    Product = PipelineProduct

    # Available parameters are a class variable. This must be present, even if empty.
    parameters = cpl.ui.ParameterList([])

    def __init__(self, recipe: 'MetisRecipe') -> None:
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
        """
            The main function of the recipe implementation, mirrors the signature of Recipe.run.
            All recipe implementations should follow this procedure schema.
        """

        try:
            self.frameset = frameset
            self.import_settings(settings)      # Import and process the provided settings dict

            self.input = self.Input(frameset)   # Create an appropriate Input object
            self.input.verify()                 # Verify that they are valid (maybe with `schema` too?)
            self.process_images()               # Do the actual processing
            self.save_products()                # Save the output products
        except cpl.core.DataNotFoundError as e:
            Msg.warning(self.__class__.__qualname__, f"Data not found: {e.message}")

        return self.build_product_frameset()      # Return the output as a pycpl FrameSet

    def import_settings(self, settings: Dict[str, Any]) -> None:
        """ Update the recipe parameters with the values requested by the user """
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(self.__class__.__qualname__,
                            f"Settings includes '{key}':{value} "
                            f"but class {self.__class__.__qualname__} "
                            f"has no parameter named {key}.")

    @abstractmethod
    def process_images(self) -> Dict[str, PipelineProduct]:
        return {}

    def save_products(self) -> None:
        """ Register the created product """
        for name, product in self.products.items():
            Msg.debug(self.__class__.__qualname__, f"Saving {product}")
            product.save()

    def build_product_frameset(self) -> cpl.ui.FrameSet:
        """ Gather all the products and build a FrameSet from their frames. """
        product_frames = cpl.ui.FrameSet()

        for name, product in self.products.items():
            product_frames.append(product.as_frame())

        return product_frames

    @property
    @abstractmethod
    def detector_name(self) -> str | None:
        """
        Return the name of the detector that is processed by this recipe.
        Default is None -- to assist in crashing your precious program.
        """
        return None


class MetisRecipe(cpl.ui.PyRecipe):
    """
        The abstract base class for all METIS recipes.
        In an ideal world it would also be abstract (metaclass=abc.ABCMeta),
        but then `pyesorex` would instantiate it on initialization and crash.
        The _fields must be present but should be overwritten by every child class.
    """
    _name = "metis_abstract_base"
    _version = "0.0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "CPL-3.0-or-later"
    _synopsis = "Abstract-like base class for METIS recipes"
    _description = "This class serves as the base class for all METIS recipes."

    parameters = cpl.ui.ParameterList([])   # By default, classes do not have any parameters
    implementation_class = str              # Dummy class, this is instantiated but not used, `str` does not hurt.

    def __init__(self):
        super().__init__()
        self.implementation = self.implementation_class(self)

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """ The main method, as required by PyCPL. It just calls the same method in the decoupled implementation. """
        return self.implementation.run(frameset, settings)
