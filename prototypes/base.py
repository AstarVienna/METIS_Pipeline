from abc import ABC, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.product import PipelineProduct
from prototypes.inputs import PipelineInputSet


class MetisRecipeImpl(ABC):
    """
        An abstract base class for all METIS recipe implementations.
        Contains central flow control and provides abstract methods to be overridden
        by particular pipeline recipe implementations.
    """
    InputSet = PipelineInputSet
    Product = PipelineProduct

    # Available parameters are a class variable. This must be present, even if empty.
    parameters = cpl.ui.ParameterList([])

    def __init__(self, recipe: 'MetisRecipe') -> None:
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.inputset = None
        self.frameset = None
        self.header = None
        self.product_frames = cpl.ui.FrameSet()
        self.products = {}

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """
            The main function of the recipe implementation. Mirrors the signature of Recipe.run.
            All recipe implementations follow this schema (and hence it does not have to be repeated).
        """

        try:
            self.frameset = frameset
            self.import_settings(settings)                # Import and process the provided settings dict
            self.inputset = self.InputSet(frameset)       # Create an appropriate Input object
            self.inputset.verify()                        # Verify that they are valid (maybe with `schema` too?)
            products = self.process_images()              # Do all the actual processing
            self.save_products(products)                  # Save the output products

            return self.build_product_frameset(products)  # Return the output as a pycpl FrameSet
        except cpl.core.DataNotFoundError as e:
            Msg.error(self.__class__.__qualname__, f"Data not found error: {e.message}")
            raise e


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
        """
        The core method of the recipe implementation. It should contain all the processing logic.
        At its entry point the Input class must be already loaded and verified.

        The basic workflow inside should be as follows:

        1.  Load the actual CPL Images associated with Input frames.
        2.  Do the preprocessing (dark, bias, flat, ...) as needed
        3.  Build the output images as specified in the DRLD.
            Each product should be an instance of the associated Product class.
        4.  Return a dictionary in the form {tag: Product(...)}

        The resulting products are passed to `save_products()`.
        """
        return {}

    def save_products(self, products: Dict[str, PipelineProduct]) -> None:
        """ Save and register the created products """
        for name, product in products.items():
            Msg.debug(self.__class__.__qualname__,
                      f"Saving {name}")
            product.save()

    def build_product_frameset(self, products: Dict[str, PipelineProduct]) -> cpl.ui.FrameSet:
        """ Gather all the products and build a FrameSet from their frames. """
        Msg.debug(self.__class__.__qualname__, f"Building the product frameset")
        product_frames = cpl.ui.FrameSet()

        for name, product in products.items():
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
        In an ideal world it would also be abstract (derived from ABC, or metaclass=abc.ABCMeta),
        but `pyesorex` wants to instantiate all recipes it finds
        and would crash with an abstract class.
        The underscored _fields must be present but should be overwritten
        by every child class (`pyesorex` actually checks for their presence).
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
        """
            The main method, as required by PyCPL.
            It just calls the same method in the decoupled implementation.
        """
        return self.implementation.run(frameset, settings)
