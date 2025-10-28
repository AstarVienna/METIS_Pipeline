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
from pymetis.classes.mixins.detector import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class BadPixMap(DetectorSpecificMixin, ImageDataItem, abstract=True):
    _title_template = "bad pixel map"
    _name_template = r'BADPIX_MAP_{detector}'
    _description_template = "Bad pixel map. Warning: may contain detector masks."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG'}

    _schema = {
        'PRIMARY': Image,
        #'DET1.DATA': Image,
    }


class BadPixMap2rg(Detector2rgMixin, BadPixMap):
    pass


class BadPixMapGeo(DetectorGeoMixin, BadPixMap):
    pass


class BadPixMapIfu(DetectorIfuMixin, BadPixMap):
    pass

    _schema = {
       'PRIMARY': None,
       'DET1.DATA': Image,
       'DET2.DATA': Image,
       'DET3.DATA': Image,
       'DET4.DATA': Image,
    }
