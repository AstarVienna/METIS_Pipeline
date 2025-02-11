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

from typing import Dict, Any

import cpl


class MetisRecipe(cpl.ui.PyRecipe):
    """
        The abstract base class for all METIS recipes.
        In an ideal world it would also be abstract (derived from ABC, or metaclass=abc.ABCMeta),
        but `pyesorex` wants to instantiate all recipes it finds
        and would crash if it were an abstract class.

        The underscored _fields must be present but should be overwritten
        by every child class (and `pyesorex` actually checks for their presence).
    """
    _name = "metis_abstract_base"
    _version = "0.0.1"
    _author = "METIS PIP team"
    _email = "astar.vienna@univie.ac.at"                        # ToDo is this a sensible default?
    _copyright = "GPL-3.0-or-later"                             # I guess we're using the same copyright everywhere
    _synopsis = "Abstract-like base class for METIS recipes"
    _description = ("This class serves as the base class for all METIS recipes."
                    "Bonus points if it is not visible from pyesorex.")

    parameters = cpl.ui.ParameterList([])           # By default, a recipe does not have any parameters.
                                                    # ToDo: There is a pyesorex bug that prevents
                                                    # this from being actually used.
    implementation_class: type["MetisRecipeImpl"]   # Dummy class, this must be overridden in the derived classes anyway

    def __init__(self):
        super().__init__()
        self.implementation: "MetisRecipeImpl" = None

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
        self.implementation = self.dispatch_implementation_class(frameset)(self)
        return self.implementation.run(frameset, settings)