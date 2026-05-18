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
from cpl.core import Image, ImageList

from pymetis.engine.dataitems import ImageDataItem
from pymetis.instruments.metis.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class LinearityMap(ImageDataItem, abstract=True):
    _name_template = r'LINEARITY_{detector}'
    _title_template = r'{detector} linearity'
    _description_template = "Coefficients for the pixel {detector} non-linearity correction"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = frozenset({'PRO.CATG'})

    _schema = {
        'PRIMARY': None,
    }


class LinearityMap2rg(Detector2rgMixin, LinearityMap):
    _schema = LinearityMap._schema | {
        'DET1.SCI': ImageList,
        'DET1.ERR': ImageList,
        'DET1.DQ': Image,
    }


class LinearityMapGeo(DetectorGeoMixin, LinearityMap):
    _schema = LinearityMap._schema | {
        'DET1.SCI': ImageList,
        'DET1.ERR': ImageList,
        'DET1.DQ': Image,
    }


class LinearityMapIfu(DetectorIfuMixin, LinearityMap):
    _schema = {
        'PRIMARY': None,
    } | {
        rf'DET{det:1d}.SCI': ImageList for det in [1, 2, 3, 4]
    } | {
        rf'DET{det:1d}.ERR': ImageList for det in [1, 2, 3, 4]
    } | {
        rf'DET{det:1d}.DQ': Image for det in [1, 2, 3, 4]
    }
