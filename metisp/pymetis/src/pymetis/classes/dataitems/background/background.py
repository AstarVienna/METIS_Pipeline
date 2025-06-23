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
                                    BandNMixin, BandLmMixin, BandSpecificMixin)

class Background(BandSpecificMixin, TargetSpecificMixin, DataItem, abstract=True):
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe

    @classmethod
    def name(cls):
        return rf"{cls.band()}_{cls.target()}_BKG_SUBTRACTED"

    @classmethod
    def title(cls):
        return rf"{cls.band()} {cls.get_target_string()} background-subtracted"

    @classmethod
    def description(cls):
        return rf"Thermal background subtracted images of {cls.get_target_string()} {cls.band()} exposures."


class LmStdBackground(BandLmMixin, TargetStdMixin, Background):
    pass


class LmSciBackground(BandLmMixin, TargetSciMixin, Background):
    pass


class NStdBackground(BandNMixin, TargetStdMixin, Background):
    pass


class NSciBackground(BandNMixin, TargetSciMixin, Background):
    pass
