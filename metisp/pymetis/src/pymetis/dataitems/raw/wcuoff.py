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
from pymetis.classes.mixins import BandLmMixin, BandNMixin, BandIfuMixin


class WcuOffRaw(ImageDataItem, abstract=True):
    """
    WCU_OFF input illuminated by the WCU up-to and including the
    integrating sphere, but no source.
    """
    _name_template = r'{band}_WCU_OFF_RAW'
    _title_template = r"{band} WCU OFF raw"
    _description_template = "Raw data for dark subtraction in other recipes."
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
        'DET2.DATA': Image,
        'DET3.DATA': Image,
        'DET4.DATA': Image,
    }


class LmWcuOffRaw(BandLmMixin, WcuOffRaw):
    pass


class NWcuOffRaw(BandNMixin, WcuOffRaw):
    pass


class IfuWcuOffRaw(BandIfuMixin, WcuOffRaw):
    _oca_keywords = WcuOffRaw._oca_keywords | {'DRS.IFU'}
