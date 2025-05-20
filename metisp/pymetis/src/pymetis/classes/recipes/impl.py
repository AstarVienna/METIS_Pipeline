"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""
import os
from abc import abstractmethod, ABC
from typing import Dict, Any, final

import cpl
from cpl.core import Msg

from pymetis.classes.products import PipelineProduct
from pymetis.classes.inputs.inputset import PipelineInputSet


class MetisRecipeImpl(ABC):
    """
    An abstract base class for all METIS recipe implementations.
    Contains central data flow control and also provides abstract methods to be overridden
    by particular pipeline recipe implementations.
    """
    InputSet: type[PipelineInputSet] = None

    # Available parameters are a class variable. This must be present, even if empty.
    parameters = cpl.ui.ParameterList([])

    def __init__(self,
                 recipe: 'MetisRecipe',
                 frameset: cpl.ui.FrameSet,
                 settings: Dict[str, Any]) -> None:
        """
        Initializes the recipe implementation object with parameters from the recipe
        and sets internal attributes to None / empty as needed.
        """
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.header: cpl.core.PropertyList | None = None
        self.products: [PipelineProduct] = []
        self.product_frames = cpl.ui.FrameSet()

        self.frameset: cpl.ui.FrameSet = frameset
        self.inputset: PipelineInputSet = self.InputSet(frameset)         # Create an appropriate InputSet object
        self.import_settings(settings)                  # Import and process the provided settings dict
        self.inputset.print_debug()
        self.inputset.validate()                        # Verify that they are valid (maybe with `schema` too?)
        self.__class__ = self._dispatch_child_class()

    def run(self) -> cpl.ui.FrameSet:
        """
            The main function of the recipe implementation. It mirrors the signature of `Recipe.run`
            and is meant to be called directly by the owner recipe.
            All recipe implementations should follow this schema
            (and hence it does not have to be repeated or overridden anywhere).

            ToDo: At least Martin thinks so now. It might change, but needs compelling arguments.
        """

        try:
            self.products = self.process_images()           # Do all the actual processing
            self._save_products()                           # Save the output products

            return self.build_product_frameset()            # Return the output as a pycpl FrameSet
        except cpl.core.DataNotFoundError as e:
            Msg.error(self.__class__.__qualname__, f"Data not found error: {e.message}")
            raise e

    def import_settings(self, settings: Dict[str, Any]) -> None:
        """
        Update the recipe parameters with the values requested by the user.
        Warn if any of the parameters is not recognized.
        """
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(self.__class__.__qualname__,
                            f"Settings include '{key}' = {value} "
                            f"but class {self.__class__.__qualname__} "
                            f"has no parameter named {key}.")

    @abstractmethod
    def process_images(self) -> [PipelineProduct]:
        """
        The core method of the recipe implementation. It should contain all the processing logic.
        At its entry point the `InputSet` class must be already loaded and validated.

        All pixel manipulation should happen inside this function (or something it calls from within).
        Put explicitly, this means
            - no pixel manipulation *before* entering `process_images`,
            - and no pixel manipulation *after* exiting `process_images`.

        The basic workflow inside this function should be as follows:

        1.  Load the CPL Images associated with `Input` frames.
        2.  Do the preprocessing (dark, bias, flat, persistence...) as needed.
            When implementing this function, please always use the topmost applicable method:
                - Use the functions provided in the pipeline if possible (derive or override).
                  Much of the functionality is common to many recipes, and we should not repeat ourselves.
                  Some classes / functions are provided in `prefab`.
                - Use HDRL functions, if available.
                - Use CPL functions, if available.
                - Implement what you need yourself.
        3.  Build the output images as specified in the DRLD.
            Each product should be an instance of the associated `PipelineProduct` class.
            There should be exactly one `PipelineProduct` for every file produced (at least for now).
        4.  Return a list of `PipelineProduct`s.

        The resulting products dict is then passed to `save_products()` (see `run`).
        """
        return []

    @final
    def _save_products(self) -> None:
        """
        Save and register the created products.
        """
        assert self.products is not None, "Products have not been created yet!"

        Msg.debug(self.__class__.__qualname__,
                  f"Saving {len(self.products)} products: {self.products}")
        for product in self.products:
            product.save()

    @final
    def build_product_frameset(self) -> cpl.ui.FrameSet:
        """
        Gather all the products and build a FrameSet from their frames so that it can be returned from `run`.
        """
        Msg.debug(self.__class__.__qualname__,
                  "Building the product frameset")
        return cpl.ui.FrameSet([product.as_frame() for product in self.products])

    @final
    def as_dict(self) -> dict[str, Any]:
        """
        Converts the object and its related data into a dictionary representation.

        Returns:
            dict[str, Any]: A dictionary that contains the serialized representation
            of the object's data, including both input set data and product data.
        """
        return {
            'title': self.name,
            'inputset': self.inputset.as_dict(),
            'products': {
                str(product.category): product.as_dict() for product in self.products
            }
        }

    @staticmethod
    def _create_dummy_header() -> cpl.core.PropertyList:
        """
        Create a dummy header (absolutely no assumptions, just to have something to work with).
        # ToDo This function should not survive in the future.
        """
        return cpl.core.PropertyList()

    @staticmethod
    def _create_dummy_image() -> cpl.core.Image:
        """
        Create a dummy image (absolutely no assumptions, just to have something to work with).
        # ToDo This function should not survive in the future.
        """
        return cpl.core.Image.load(os.path.expandvars("$SOF_DATA/LINEARITY_2RG.fits"))

    @staticmethod
    def _create_dummy_table() -> cpl.core.Table:
        """
        Create a dummy table (absolutely no assumptions, just to have something to work with).
        # ToDo This function should not survive in the future.
        """
        return cpl.core.Table.empty(3)

    @property
    def valid_frames(self) -> cpl.ui.FrameSet:
        return self.inputset.valid_frames

    @property
    def used_frames(self) -> cpl.ui.FrameSet:
        return self.inputset.used_frames

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        """
        Return the actual implementation class **when the frameset is already available**, e.g. at runtime.
        The base implementation just returns its own class, so nothing happens,
        but more complex recipes may need to select the appropriate derived class based on the input data.

        Typical use is to accomodate for different implementations for
            - multiple detectors (2RG|GEO|IFU)
            - different targets (STD|SCI)
            - different bands (LM|N)
        or similar.

        It should be something along these lines:
        ```
        return {
            'STD': ChildClassStd,
            'SCI': ChildClassSci,
        }[self.inputset.target]
        ```
        or use a proper match ... case ... structure if appropriate.
        """
        return self.__class__
