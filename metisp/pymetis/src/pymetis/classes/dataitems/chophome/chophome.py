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

from pymetis.classes.dataitems.dataitem import ImageDataItem
from pymetis.classes.mixins.band import BandLmMixin


class LmChophomeCombined(BandLmMixin, ImageDataItem):
    _name_template = r'LM_CHOPHOME_COMBINED'
    _title_template = "LM chop-home combined"
    _description_template = "Stacked LM band exposures."
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI20.NAME'}


class LmChophomeBackground(BandLmMixin, ImageDataItem):
    _name_template = r'LM_CHOPHOME_BACKGROUND'
    _title_template = "LM chop-home background"
    _description_template = "Stacked WCU background images."
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI19.NAME', 'INS.OPTI20.NAME'}