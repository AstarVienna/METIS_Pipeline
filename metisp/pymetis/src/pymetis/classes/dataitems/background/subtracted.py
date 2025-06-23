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

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import (TargetSpecificMixin, TargetSciMixin, TargetStdMixin,
                                    BandSpecificMixin, BandLmMixin, BandNMixin)


class BackgroundSubtracted(BandSpecificMixin, TargetSpecificMixin, DataItem, abstract=True):
    _title = "background-subtracted"
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe

    @classmethod
    def name(cls):
        return rf"{cls.band()}_{cls.target()}_BKG_SUBTRACTED"

    @classmethod
    def description(cls):
        return rf"Thermal background subtracted images of science {cls.band()} {cls.target()} exposures."


# ToDo: generate this classes automatically
class LmStdBackgroundSubtracted(BandLmMixin, TargetStdMixin, BackgroundSubtracted):
    pass


class LmSciBackgroundSubtracted(BandLmMixin, TargetSciMixin, BackgroundSubtracted):
    pass


class NStdBackgroundSubtracted(BandNMixin, TargetStdMixin, BackgroundSubtracted):
    pass


class NSciBackgroundSubtracted(BandNMixin, TargetSciMixin, BackgroundSubtracted):
    pass
