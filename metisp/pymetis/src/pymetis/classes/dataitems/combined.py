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

from pymetis.classes.dataitems import DataItem
from pymetis.classes.mixins import (BandSpecificMixin, BandLmMixin, BandIfuMixin,
                                    TargetSpecificMixin, TargetStdMixin, TargetSciMixin)

"""
The hierarchy is somewhat atypical here by design: no N data item, and only IFU support STD|SKY target.
"""

class Combined(BandSpecificMixin, DataItem, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    @classmethod
    def name(cls) -> str:
        return rf'{cls.band()}_STD_COMBINED'

    @classmethod
    def description(cls) -> str:
        return f"Stacked {cls.band()} band exposures."


class LmStdCombined(BandLmMixin, Combined):
    pass