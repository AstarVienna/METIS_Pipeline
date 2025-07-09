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

from pymetis.classes.dataitems.raw import Raw
from pymetis.classes.mixins.detector import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class DarkRaw(DetectorSpecificMixin, Raw, abstract=True):
    _name_template = r'DARK_{detector}_RAW'
    _title_template = r'{detector} dark raw'
    _description_template = r"Raw data for creating a {detector} master dark."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE', 'DET.ID', 'DET.DIT'}


class Dark2rgRaw(Detector2rgMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.FILTER'}


class DarkGeoRaw(DetectorGeoMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.FILTER'}


class DarkIfuRaw(DetectorIfuMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.IFU'}