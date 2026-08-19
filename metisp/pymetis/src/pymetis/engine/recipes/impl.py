"""
This file is part of an A* Pipeline.
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
from abc import abstractmethod, ABC
from typing import Dict, Any, final, Optional, TYPE_CHECKING

import cpl
from cpl.core import Msg

from pymetis.engine.core.parameter import ParameterList
from pymetis.engine.core.parametrizable import Parametrizable

from pymetis.engine.dataitems import DataItem, PipelineProductSet
from pymetis.engine.inputs.inputset import PipelineInputSet
from pymetis.engine.qc import QcParameterSet, QcParameter

if TYPE_CHECKING:
    from pymetis.engine.recipes.recipe import Recipe


class RecipeImpl(Parametrizable, ABC):
    """
    An abstract base class for all recipe implementations.
    Contains central data flow control and also provides abstract methods to be overridden
    by particular pipeline recipe implementations.
    """
    InputSet: Optional[type[PipelineInputSet]] = None
    ProductSet: Optional[type[PipelineProductSet]] = PipelineProductSet
    Qc: Optional[type[QcParameterSet]] = QcParameterSet

    # Available parameters are a class variable. This must be present, even if empty.
    parameters = ParameterList([])

    def __init__(self,
                 recipe: 'Recipe',
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
        self.products: set[DataItem] = set()
        self.product_frames = cpl.ui.FrameSet()

        self.frameset: cpl.ui.FrameSet = frameset
        self.inputset: PipelineInputSet = self.InputSet(frameset)         # Create an appropriate InputSet object
        self.inputset.validate()                        # Verify that they are valid (maybe with `schema` too?)

        # Promote the products and QC parameters to the correct subclasses, based on
        # - tag parameters (from defined mixins, class-based)
        # - tag matches (from the loaded frameset, instance-based)
        # The promoted containers are assigned to the *instance* (shadowing the class
        # attributes), so runs of the same recipe never affect one another.
        # ToDo: Decide what to do in case of a conflict between those two
        tags = self.tag_parameters() | self.inputset.tag_matches
        try:
            self.ProductSet = self.ProductSet.promoted(**tags)
            self.Qc = self.Qc.promoted(**tags)
        except TypeError as e:
            Msg.error(self.__class__.__qualname__, str(e))
            raise
        self.import_settings(settings)                  # Import and process the provided settings dict
        self.inputset.print_debug()
        Msg.debug(self.__class__.__qualname__,
                  f"{'-' * 40} Recipe initialization complete {'-' * 40}")

    @classmethod
    def specialize(cls) -> None:
        """
        Specialize the recipe implementation to the current class parameters.

        Each Impl gets its own private ProductSet / Qc subclass exactly once, so that
        specializing it cannot leak into a prefab parent shared with sibling recipes,
        and repeated calls (e.g. repeated man-page rendering) do not grow the MRO.
        """
        Msg.debug(cls.__qualname__, f"Specializing Implementation {cls.__qualname__} with {cls.tag_parameters()}")

        for name in ("ProductSet", "Qc"):
            if name not in cls.__dict__:
                container = type(name, (getattr(cls, name),), {})
                container.__qualname__ = f"{cls.__qualname__}.{name}"
                container.__module__ = cls.__module__
                setattr(cls, name, container)
            getattr(cls, name).specialize(**cls.tag_parameters())

    def run(self) -> cpl.ui.FrameSet:
        """
        The main function of the recipe implementation. It mirrors the signature of `Recipe.run`
        and is meant to be called directly by the owner recipe.
        All recipe implementations should follow this schema
        (and hence it does not have to be repeated or overridden anywhere).

        ToDo: At least Martin thinks so now. It might change, but needs compelling arguments.
        """

        try:
            self.products: set[DataItem] = self.process()   # Do all the actual processing
            self._save_products()                           # Save the output products

            return self.build_product_frameset()            # Return the output as a pycpl FrameSet
        except cpl.core.DataNotFoundError as e:
            Msg.error(self.__class__.__qualname__,
                      f"Data not found error: {e.message}")
            raise e
        except Exception as e:
            Msg.error(self.__class__.__qualname__,
                      f"Unexpected exception occurred: {e}")
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
    def process(self) -> set[DataItem]:
        """
        The core method of the recipe implementation. It should contain all the processing logic.
        At its entry point, the `InputSet` class must be already loaded and validated.
        This should not be a concern of the author of a recipe, as long as everything is declared correctly.

        All pixel manipulation should happen inside this function (or private subroutines it calls from within).
        Put explicitly, this means
            - no pixel manipulation *before* entering `process`,
            - and no pixel manipulation *after* exiting `process`.

        The basic workflow inside this function should be as follows:

        1.  Load the CPL structures associated with `PipelineInput` frames.
            To conserve resources, most importantly memory, defer the `load_data` call
            until the data are actually needed.
        2.  Do the preprocessing (dark, bias, flat, persistence...) as needed.
            When implementing this function, please always use the topmost applicable method:
                - Use the functions provided by the pipeline package if possible (derive or override).
                  Much of the functionality is trivially common to many recipes, and we should not repeat ourselves.
                  Some classes / functions are provided in ``prefab``.
                - Use HDRL functions, if available.
                - Use CPL functions, if available.
                - Implement what you need yourself as a subroutine.
        3.  Build the output images as specified in the DRLD.
            Each product should be a ``DataItem`` and there should be exactly one for every file produced.
        4.  Return a set of ``DataItem`` instance.

        The resulting products set is then passed to `save_products()` (see `run`).
        """
        return set()

    def collect_qc_parameters(self, *qc_parameters: QcParameter) -> cpl.core.PropertyList:
        """
        Collect an *args list of QC parameters and convert them to a PropertyList
        """
        out = cpl.core.PropertyList()
        for qcparam in qc_parameters:
            out.append(qcparam.as_property())

        return out

    @final
    def _save_products(self) -> None:
        """
        Save and register the created products.
        """
        assert self.products is not None, "Products have not been created yet!"

        Msg.debug(self.__class__.__qualname__,
                  f"Saving {len(self.products)} products:")
        for product in self.products:
            Msg.debug(self.__class__.__qualname__,
                      f"   {product.name():<40} {product._get_file_name()}")
            product.save(recipe=self, parameters=self.parameters)

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
                str(product.name()): product.as_dict() for product in self.products
            }
        }

    @property
    def valid_frames(self) -> cpl.ui.FrameSet:
        return self.inputset.valid_frames

    @property
    def used_frames(self) -> cpl.ui.FrameSet:
        return self.inputset.used_frames