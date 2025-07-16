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

import cpl.ui

from pymetis.dataitems.dataitem import TableDataItem
from pymetis.classes.mixins import BandSpecificMixin, BandLmMixin, BandNMixin, BandIfuMixin


class DistortionReduced(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template =  r'{band}_DIST_REDUCED'
    _title_template = r"{band} distortion reduced"
    _description_template = r"Table of polynomial coefficients for distortion correction"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG'}


class LmDistortionReduced(BandLmMixin, DistortionReduced):
    _oca_keywords = DistortionReduced._oca_keywords | {'DRS.FILTER'}


class NDistortionReduced(BandNMixin, DistortionReduced):
    _oca_keywords = DistortionReduced._oca_keywords | {'DRS.FILTER'}


class IfuDistortionReduced(BandIfuMixin, DistortionReduced):
    _oca_keywords = DistortionReduced._oca_keywords | {'DRS.IFU'}
