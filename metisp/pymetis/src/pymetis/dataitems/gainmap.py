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
from cpl.core import Image

from pymetis.classes.dataitems import ImageDataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, DetectorSpecificMixin


class GainMap(DetectorSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'GAIN_MAP_{detector}'
    _title_template = "gain map"
    _description_template = "Gain map"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG'}

    _schema = {
        'PRIMARY': None,
    }


class GainMap2rg(Detector2rgMixin, GainMap):
    _schema = GainMap._schema | {
        'DET1.SCI': Image,
        #ToDo ERR and DQ to follow
    }


class GainMapGeo(DetectorGeoMixin, GainMap):
    _schema = GainMap._schema | {
        'DET1.SCI': Image,
        #ToDo ERR and DQ to follow
    }


class GainMapIfu(DetectorIfuMixin, GainMap):
    _schema = {
        'PRIMARY': None,
        'DET1.SCI': Image,
        'DET2.SCI': Image,
        'DET3.SCI': Image,
        'DET4.SCI': Image,
    }
