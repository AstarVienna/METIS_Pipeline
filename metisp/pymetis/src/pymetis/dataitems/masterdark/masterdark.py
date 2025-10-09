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
from pymetis.classes.mixins import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class MasterDark(DetectorSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'MASTER_DARK_{detector}'
    _title_template = r"{detector} master dark"
    _description_template = "Master dark frame for {detector} data"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
        'NOISE': Image,
        'BPM': Image,
    }


class MasterDark2rg(Detector2rgMixin, MasterDark):
    pass


class MasterDarkGeo(DetectorGeoMixin, MasterDark):
    pass


class MasterDarkIfu(DetectorIfuMixin, MasterDark):
    pass
