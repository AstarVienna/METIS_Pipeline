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
from abc import ABC, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg

from pymetis.base.product import PipelineProduct
from pymetis.inputs.inputset import PipelineInputSet


class MetisRecipeImpl(ABC):
    """
    An abstract base class for all METIS recipe implementations.
    Contains central data flow control and also provides abstract methods to be overridden
    by particular pipeline recipe implementations.
    """
    InputSet: type[PipelineInputSet] = None
    Product: type[PipelineProduct] = None

    # Available parameters are a class variable. This must be present, even if empty.
    parameters = cpl.ui.ParameterList([])

    def __init__(self, recipe: 'MetisRecipe') -> None:
        super().__init__()
        self.name = recipe.name
        self.version = recipe.version
        self.parameters = recipe.parameters

        self.inputset: PipelineInputSet | None = None
        self.frameset = None
        self.header = None
        self.products: Dict[str, PipelineProduct] = {}
        self.product_frames = cpl.ui.FrameSet()

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """
            The main function of the recipe implementation. It mirrors the signature of `Recipe.run`
            and is meant to be called directly by the owner recipe.
            All recipe implementations should follow this schema
            (and hence it does not have to be repeated or overridden anywhere).

            ToDo: At least Martin thinks so now. It might change, but needs compelling arguments.
            ToDo: If this structure does not cover the needs of your particular recipe, we should discuss and adapt.
        """

        try:
            self.frameset = frameset
            self.import_settings(settings)                # Import and process the provided settings dict
            self.inputset = self.InputSet(frameset)       # Create an appropriate InputSet object
            self.inputset.print_debug()
            self.inputset.validate()                      # Verify that they are valid (maybe with `schema` too?)
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
        At its entry point the Input class must be already loaded and validated.

        All pixel manipulation should happen inside this function (or something it calls from within).
        Put explicitly,
            - no pixel manipulation before entering `process_images`,
            - and no pixel manipulation after exiting `process_images`.

        The basic workflow inside this function should be as follows:

        1.  Load the actual CPL Images associated with Input frames.
        2.  Do the preprocessing (dark, bias, flat, persistence...) as needed.
            When implementing this function, please use the topmost applicable method:
                - Use the functions provided in the pipeline if possible (derive or override).
                  Much of the functionality is common to many recipes, and we should not repeat ourselves.
                - Use HDRL functions, if available.
                - Use CPL functions, if available.
                - Implement what you need yourself.
        3.  Build the output images as specified in the DRLD.
            Each product should be an instance of the associated `Product` class.
            There should be exactly one `Product` for every file produced (at least for now).
        4.  Return a dictionary in the form {tag: ProductTag(...)}

        The resulting products dict is then passed to `save_products()` (see `run`).
        """
        return {}

    def save_products(self, products: Dict[str, PipelineProduct]) -> None:
        """
        Save and register the created products.
        """
        for name, product in products.items():
            Msg.debug(self.__class__.__qualname__,
                      f"Saving product {name}")
            product.save()

    def build_product_frameset(self, products: Dict[str, PipelineProduct]) -> cpl.ui.FrameSet:
        """
        Gather all the products and build a FrameSet from their frames so that it can be returned from `run`.
        """
        Msg.debug(self.__class__.__qualname__,
                  f"Building the product frameset")
        return cpl.ui.FrameSet([product.as_frame() for product in products.values()])

    def as_dict(self) -> dict[str, Any]:
        """
        Converts the object and its related data into a dictionary representation.

        Return:
            dict[str, Any]: A dictionary that contains the serialized representation
            of the object's data, including both input set data and product data.
        """
        return {
            'title': self.name,
            'inputset': self.inputset.as_dict(),
            'products': {
                product.tag: product.as_dict() for product in self.products.values()
            }
        }

    @staticmethod
    def _create_dummy_header():
        """
        Create a dummy header (absolutely no assumptions, just to have something to work with).
        This function should not survive in the future.
        """
        return cpl.core.PropertyList()

    @staticmethod
    def _create_dummy_image():
        """
        Create a dummy image (absolutely no assumptions, just to have something to work with).
        This function should not survive in the future.
        """
        return cpl.core.Image.load(os.path.expandvars("$SOF_DATA/LINEARITY_2RG.fits"))