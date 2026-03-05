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

from pymetis.dataitems.raw import Raw
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class LinearityRaw(Raw, abstract=True):
    _name_template = r'DETLIN_{detector}_RAW'
    _title_template = r'{detector} linearity raw'
    _description_template = r"Raw data for non-linearity determination for {detector} observations"
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE'}


class LinearityRaw2rg(Detector2rgMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.FILTER'}


class LinearityRawGeo(DetectorGeoMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.FILTER'}


class LinearityRawIfu(DetectorIfuMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.IFU'}