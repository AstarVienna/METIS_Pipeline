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

from pymetis.dataitems.dataitem import ImageDataItem
from pymetis.classes.mixins import (TargetSpecificMixin, TargetSciMixin, TargetStdMixin,
                                    BandNMixin, BandLmMixin, BandSpecificMixin)

class Background(BandSpecificMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_{target}_BKG'
    _title_template = r'{band}_{target} background, subtracted'
    _description_template = r"Thermal background subtracted images of {target} {band} exposures."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}


class LmStdBackground(BandLmMixin, TargetStdMixin, Background):
    pass


class LmSciBackground(BandLmMixin, TargetSciMixin, Background):
    pass


class NStdBackground(BandNMixin, TargetStdMixin, Background):
    pass


class NSciBackground(BandNMixin, TargetSciMixin, Background):
    pass
