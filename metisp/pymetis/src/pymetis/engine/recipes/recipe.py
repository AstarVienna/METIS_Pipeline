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
import inspect
import re
from typing import Any, Generator, Self, ClassVar

import cpl
from astropy.utils import classproperty

from ..core.parameter import ParameterList
from ..dataitems import DataItem
from ..qc import QcParameter
from ..recipes.impl import RecipeImpl
from ..inputs import PipelineInput


class Recipe(cpl.ui.PyRecipe):
    """
    The abstract base class for all recipes.
    In an ideal world it would also be abstract (derived from ABC, or metaclass=abc.ABCMeta),
    but `pyesorex` tries to instantiate all recipes it finds and would crash if it were an abstract class.

    The underscored _fields must be present but should be overwritten
    by every child class (`pyesorex` actually checks for their presence).
    """

    # Seven mandatory attributes follow. These are required by pyesorex and not negotiable.
    # Some of them do not have to be overwritten as they are always the same.
    _name: str = "recipe_abstract_base"
    _version: str = "0.0.1"
    _author: str = "A* PIP team"
    _email: str = "astar.vienna@univie.ac.at"                    # ToDo is this a sensible default?
    _copyright: str = "GPL-3.0-or-later"                         # I guess we are using the same copyright everywhere
    _synopsis: str = "Abstract-like base class for A* recipes"
    _description: str = ("This class serves as the base class for all A* recipes. "
                         "If you see this in a recipe, override its `_description` attribute.")

    # More internal attributes follow. These are **not** required by pyesorex and are specific to A*.
    _matched_keywords: frozenset[str] = None
    # Verbal description of the algorithm
    _algorithm: str = "<no algorithm provided>"
    # DRLD traceability: requirement IDs and observation template names.
    # These are not used at runtime but are consumed by the autorecipe Sphinx
    # extension to generate the DRLD JSON manifest and recipe reference pages.
    _requirements: set[str] = set()
    _templates: set[str] = set()

    # By default, a recipe does not have any parameters.
    parameters: ParameterList = ParameterList([])
    # Default implementation class. This will not work, because it is abstract, but this is an abstract class too.
    Impl: type[RecipeImpl] = RecipeImpl

    _registry: ClassVar[dict[str, type[Self]]] = {}

    def __init__(self):
        super().__init__()
        # Build a fancy description from attributes
        self.implementation: RecipeImpl | None = None

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls._description: str = cls._build_description()
        cls._registry[cls._name] = cls

    @classproperty
    def description(cls) -> str:
        return cls._description

    def run(self, frameset: cpl.ui.FrameSet, settings: dict[str, Any]) -> cpl.ui.FrameSet:
        """
        The main method, as required by PyCPL.
        Instantiates the decoupled implementation, fills it with the supplied frameset,
        optionally promotes the class to the proper derived class and then runs it.
        """
        self.implementation = self.Impl(self, frameset, settings)
        return self.implementation.run()

    @classmethod
    def _list_dataitems_input(cls, dataitem_class: type[DataItem]) -> Generator[Self, None, None]:
        """
        List all Recipe classes that use a particular DataItem as an input.
        Warning: heavy introspection.
        Useful for reconstruction of DRLD input/product cards.
        """
        for (name, klass) in cls._registry.items():
            for (n, kls) in inspect.getmembers(klass.Impl.InputSet,
                                               lambda x: (inspect.isclass(x) and issubclass(x, PipelineInput))):
                if issubclass(kls.Item, dataitem_class):
                    yield klass

    @classmethod
    def _list_dataitems_product(cls, dataitem_class: type[DataItem]) -> Generator[Self, None, None]:
        """
        List all Recipe classes that have a particular DataItem as a product.
        Warning: heavy introspection.
        Useful for reconstruction of DRLD input/product cards.
        """
        for (name, klass) in cls._registry.items():
            for (n, kls) in inspect.getmembers(klass.Impl.ProductSet,
                                               lambda x: inspect.isclass(x)):
                if issubclass(kls, dataitem_class):
                    yield klass

    @classmethod
    def _list_inputs(cls) -> list[tuple[str, type[PipelineInput]]]:
        return cls.Impl.InputSet.list_input_classes()

    @classmethod
    def _list_products(cls) -> list[tuple[str, type[DataItem]]]:
        return cls.Impl.ProductSet.list_classes()

    @classmethod
    def _list_qc_parameters(cls) -> list[tuple[str, type[QcParameter]]]:
        return cls.Impl.Qc.list_classes()

    @staticmethod
    def _format_spacing(text: str, title: str, offset: int = 4) -> str:
        """ A kludgy attempt to format the algorithm description to have nice indentation. """

        # First, remove all spaces from the beginning of the string.
        fix_first_space = re.compile(r'^\s*')
        # Second, remove all spaces from the beginning of each line and a fixed number of spaces.
        fix_spacing = re.compile(r'\n\s*')

        return fix_spacing.sub('\n' + ' ' * offset, fix_first_space.sub(' ' * offset, text)) \
            if text is not None else f'<no {title} defined>'

    @classmethod
    def _build_description(cls) -> str:
        """
        Automatically build the `description` attribute from available attributes.
        Everything inside this should only depend on the class, never on an instance.
        """
        if cls._matched_keywords is None:
            matched_keywords = '<not defined>'
        elif len(cls._matched_keywords) == 0:
            matched_keywords = '--- none ---'
        else:
            matched_keywords = '\n  '.join(cls._matched_keywords)

        cls.Impl.specialize()

        inputs = '\n'.join(sorted([input_type.extended_description_line(name)
                                   for (name, input_type) in cls._list_inputs()]))
        products = cls._format_spacing(cls.Impl.ProductSet.list_descriptions(), 'products', 2)
        qc_parameters = cls._format_spacing(cls.Impl.Qc.list_descriptions(), 'QC parameters', 2)
        algorithm = cls._format_spacing(cls._algorithm, 'algorithm', 2)
        description = cls._format_spacing(cls._description, 'description', 0)

        return f"""{cls._synopsis}\n\n{description}\n
Matched keywords
  {matched_keywords}
Inputs\n{inputs}
Outputs\n{products}
QC parameters\n{qc_parameters}
Algorithm\n{algorithm}
"""

    @property
    def algorithm(self):
        return self._algorithm