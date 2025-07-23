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

from pymetis.classes.dataitems import DataItem, ImageDataItem, TableDataItem
from pymetis.classes.mixins import DetectorIfuMixin


class Rsrf(DataItem):
    _name_template = r'RSRF'
    _title_template = "RSRF"
    _description_template = "2D relative spectral response function"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class RsrfIfu(DetectorIfuMixin, TableDataItem):
    _name_template = r'RSRF_IFU'
    _title_template = "RSRF IFU"
    _description_template = "1D relative spectral response function"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE


class IfuRsrfBackground(DetectorIfuMixin, ImageDataItem):
    _name_template = r'IFU_RSRF_BACKGROUND'
    _title_template = "IFU RSRF background"
    _description_template = "2D relative spectral response function background"
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
