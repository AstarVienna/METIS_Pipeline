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

import re
from typing import Any

import cpl

from pyesorex.parameter import ParameterList

from ..dataitems import DataItem
from ..qc import QcParameter
from pymetis.classes.recipes.impl import MetisRecipeImpl
from pymetis.classes.inputs import PipelineInput


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
    _name: str = "metis_abstract_base"
    _version: str = "0.0.1"
    _author: str = "METIS PIP team, A*"
    _email: str = "astar.vienna@univie.ac.at"                    # ToDo is this a sensible default?
    _copyright: str = "GPL-3.0-or-later"                         # I guess we are using the same copyright everywhere
    _synopsis: str = "Abstract-like base class for METIS recipes"
    _description: str = ("This class serves as the base class for all METIS recipes. "
                         "Bonus points if it is not visible from pyesorex "
                         "(if it is, override the _description attribute in the final class).")

    # More internal attributes follow. These are **not** required by pyesorex and are specific to METIS / A*.
    _matched_keywords: set[str] = set()
    _algorithm: str = None # Verbal description of the algorithm

    # By default, a recipe does not have any parameters.
    parameters: ParameterList = ParameterList([])
    # Default implementation class. This will not work, because it is abstract, but this is an abstract class too.
    Impl: type[MetisRecipeImpl] = MetisRecipeImpl

    def __init__(self):
        super().__init__()
        # Build a fancy description from attributes
        self._description: str = self._build_description()
        self.implementation: MetisRecipeImpl | None = None

    def run(self, frameset: cpl.ui.FrameSet, settings: dict[str, Any]) -> cpl.ui.FrameSet:
        """
        The main method, as required by PyCPL.
        Instantiates the decoupled implementation, fills it with supplied frameset,
        optionally promotes the class to the proper child class and then runs it.
        """
        self.implementation = self.Impl(self, frameset, settings)
        return self.implementation.run()

    def _list_inputs(self) -> list[tuple[str, type[PipelineInput]]]:
        return self.Impl.InputSet.list_input_classes()

    def _list_products(self) -> list[tuple[str, type[DataItem]]]:
        return self.Impl.ProductSet.list_classes()

    def _list_qc_parameters(self) -> list[tuple[str, type[QcParameter]]]:
        return self.Impl.Qc.list_classes()

    @staticmethod
    def _format_spacing(text: str, title: str, offset: int = 4) -> str:
        """
        A kludgy attempt to format the algorithm description to have nice indentation.
        """

        # First, remove all spaces from the beginning of the string.
        fix_first_space = re.compile(r'^\s*')
        # Second, remove all spaces from the beginning of each line and a fixed number of spaces.
        fix_spacing = re.compile(r'\n\s*')

        return fix_spacing.sub('\n' + ' ' * offset, fix_first_space.sub(' ' * offset, text)) \
            if text is not None else f'<no {title} defined>'

    def _build_description(self) -> str:
        """
        Automatically build the `description` attribute from available attributes.
        Everything inside this should only depend on the class, never on an instance.
        """
        if self._matched_keywords is None:
            matched_keywords = '<not defined>'
        elif len(self._matched_keywords) == 0:
            matched_keywords = '--- none ---'
        else:
            matched_keywords = '\n    '.join(self._matched_keywords)

        self.Impl.specialize()

        if len(qcs := self._list_qc_parameters()) == 0:
            qc_parameters = '    --- none ---'
        else:
            qc_parameters = '\n'.join([qcp_type.extended_description_line() for (name, qcp_type) in qcs])

        inputs = '\n'.join(sorted([input_type.extended_description_line(name)
                                   for (name, input_type) in self._list_inputs()]))
        products = '\n'.join(sorted([product_type.extended_description_line(name)
                                     for (name, product_type) in self._list_products()]))
        algorithm = self._format_spacing(self._algorithm, 'algorithm', 4)
        description = self._format_spacing(self._description, 'description', 2)

        return f"""{self.synopsis}\n\n{description}\n
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
