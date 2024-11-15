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

# Basic reusable mixin classes
from .masterdark import MasterDarkInputMixin
from .masterflat import MasterFlatInputMixin
from .linearity import LinearityInputMixin
from .badpixmap import BadpixMapInputMixin
from .persistence import PersistenceInputMixin
from .gainmap import GainMapInputMixin, GainMap2rgInputMixin

"""
    Mixins are classes that provide some easily includible optional functionality.
    Here we use them for two purposes:
    
     1. Adding a common element to a pipeline recipe input, such as a bad pixel map or linearity correction.
        Such blocks should provide generally the same functionality in every recipe (however the actual implementation
        is currently left for the RecipeImpl class, the Input only fetches and filters input data,
        but does not care about *what to do* with them).
        
        Due to the way Python finds the correct ordering of calls to parent methods and what super() does,
        the order of inclusion of mixins is somewhat important: the always come **before** the actual ancestors
        of the Input class, and generally they come **in the order of invocation** during the resolution
        of Input prerequisites.
        
        See also: C3 linearization
        
     2. Defining the detectors that are covered by the particular flavour of the recipe.
        For instance MetisDetDark is in fact a class of three recipes: Metis2rgDark, MetisGeoDark and MetisIfuDark,
        however the required functionality is vastly similar, except for accepted tags and such.
        
        Instead of writing every class from scratch, we can specify a master abstract class MetisDetDark,
        and then create thin wrappers for the three children. For instance Metis2rgDark is then defined
        as Metis2rgDark(Detector2rgMixin, MetisDetDark).
"""

# Mixins that set the detector name (and any related data if needed)
from .detectors import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin