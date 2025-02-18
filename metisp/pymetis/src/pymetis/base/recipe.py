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

import inspect
from typing import Dict, Any

import cpl

from pymetis.base.product import PipelineProduct
from pymetis.base.impl import MetisRecipeImpl
from pymetis.inputs import PipelineInput, SinglePipelineInput


class MetisRecipe(cpl.ui.PyRecipe):
    """
        The abstract base class for all METIS recipes.
        In an ideal world it would also be abstract (derived from ABC, or metaclass=abc.ABCMeta),
        but `pyesorex` tries to instantiate all recipes it finds and would crash if it were an abstract class.

        The underscored _fields must be present but should be overwritten
        by every child class (`pyesorex` actually checks for their presence).
    """

    # Seven mandatory attributes follow. These are required by pyesorex and not negotiable.
    # Some of them do not have to be overwritten as they are always the same.
    _name = "metis_abstract_base"
    _version = "0.0.1"
    _author = "METIS PIP team"
    _email = "astar.vienna@univie.ac.at"                        # ToDo is this a sensible default?
    _copyright = "GPL-3.0-or-later"                             # I guess we're using the same copyright everywhere
    _synopsis = "Abstract-like base class for METIS recipes"
    _description = ("This class serves as the base class for all METIS recipes."
                    "Bonus points if it is not visible from pyesorex.")

    # More internal attributes follow. These are **not** required by pyesorex and are specific to METIS / A*.
    _algorithm = "Foo the bar and quux the baz [DEFAULT]."      # Verbal description of the algorithm

    # By default, a recipe does not have any parameters.
    parameters: cpl.ui.ParameterList = cpl.ui.ParameterList([])
    # Default implementation class. This will not work, because it's abstract, but this is an abstract too.
    implementation_class: type[MetisRecipeImpl] = MetisRecipeImpl

    def __init__(self):
        super().__init__()
        self._description = self._build_description()
        self.implementation: MetisRecipeImpl | None = None

    def dispatch_implementation_class(self, frameset) -> type["MetisRecipeImpl"]:
        """
        Return the actual implementation class. By default, just returns `implementation_class`,
        but more complex recipes may need to select the appropriate class based on the frameset.
        """
        return self.implementation_class

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """
            The main method, as required by PyCPL.
            It just calls the same method in the decoupled implementation.
        """
        self.implementation = self.implementation_class(self)
        self.implementation.__class__ = self.dispatch_implementation_class(frameset)
        return self.implementation.run(frameset, settings)

    def _build_description(self):
        """
        Automatically build the `description` attribute from available data.
        """
        inputs = '\n        '.join(
            [f"{input_type._pretty_tags():<30}"
             f"[{' 1 ' if issubclass(input_type, SinglePipelineInput) else '1-n'}]"
             f"{' (optional)' if not input_type._required else '           '} "
             f"{input_type._description}"
            for (name, input_type) in
             inspect.getmembers(self.implementation_class.InputSet,
                                lambda x: inspect.isclass(x) and issubclass(x, PipelineInput))
        ])
        products = '\n        '.join(
            [f"{str(typ.tag()):<47}{typ.description}"
             for (name, typ) in
             inspect.getmembers(self.implementation_class,
                                lambda x: inspect.isclass(x) and issubclass(x, PipelineProduct))
        ])
        return \
f"""
    {self.synopsis}

    Matched keywords
        (none)
    Inputs
        {inputs}
    Outputs
        {products}
    Algorithm
        {self.algorithm}
"""

    @property
    def algorithm(self):
        return self._algorithm
