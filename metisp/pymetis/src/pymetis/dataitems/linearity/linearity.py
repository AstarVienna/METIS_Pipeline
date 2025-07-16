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

import cpl

from pymetis.classes.dataitem import ImageDataItem
from pymetis.classes.mixins import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class LinearityMap(DetectorSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'LINEARITY_{detector}'
    _title_template = r'{detector} linearity'
    _description_template = "Coefficients for the pixel {detector} non-linearity correction"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG'}


class LinearityMap2rg(Detector2rgMixin, LinearityMap):
    pass


class LinearityMapGeo(DetectorGeoMixin, LinearityMap):
    pass


class LinearityMapIfu(DetectorIfuMixin, LinearityMap):
    pass
