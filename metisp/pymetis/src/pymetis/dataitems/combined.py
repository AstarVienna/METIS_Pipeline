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

from pymetis.dataitems import ImageDataItem
from pymetis.classes.mixins import (BandSpecificMixin, BandLmMixin, BandNMixin)

"""
The hierarchy is somewhat atypical here by design: no N data item, and only IFU supports STD|SKY target.
"""

class Combined(BandSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_STD_COMBINED'
    _title_template = r'band standard combined'
    _description_template = r"Stacked {band} band exposures."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}


class LmStdCombined(BandLmMixin, Combined):
    pass


class NStdCombined(BandNMixin, Combined):
    pass