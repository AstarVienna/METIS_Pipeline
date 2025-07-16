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

from pymetis.dataitems.dataitem import DataItem
from pymetis.classes.mixins import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin


class BackgroundReduced(DetectorSpecificMixin, DataItem, abstract=True):
    _name_template = r'{detector}_BACKGROUND_REDUCED'
    _title = "background-reduced"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}


class BasicReduced(DetectorSpecificMixin, DataItem, abstract=True):
    _name_template = r'{band}_SCI_BASIC_REDUCED'


class StdBasicReduced(Detector2rgMixin, BackgroundReduced):
    _description_template = "Standard detrended exposure of the LM image mode."
    _tag = rf"LM_STD_BASIC_REDUCED"


class SciBasicReducedGeo(DetectorGeoMixin, BackgroundReduced):
    _description_template = "Science grade detrended exposure of the LM image mode."


class SkyBasicReduced(DataItem):
    _title_template = "sky basic-reduced exposure"
    _description_template = "Detrended exposure of the sky."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe