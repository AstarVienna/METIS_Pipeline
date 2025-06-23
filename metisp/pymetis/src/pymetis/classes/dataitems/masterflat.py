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

from abc import ABC

import cpl

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, BandSpecificMixin, \
    TargetSpecificMixin


class MasterFlat(DataItem, abstract=True):
    _title = r"master flat"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "Abstract base class for master flats. Please subclass."
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    @classmethod
    def name(cls) -> str:
        return rf"MASTER_FLAT_{cls.detector()}"


class MasterFlat2rg(Detector2rgMixin, MasterFlat):
    pass


class MasterFlatGeo(DetectorGeoMixin, MasterFlat):
    pass


class MasterFlatIfu(DetectorIfuMixin, MasterFlat):
    pass


class MasterImgFlat(BandSpecificMixin, TargetSpecificMixin, DataItem, abstract=True):
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

    @classmethod
    def name(cls):
        return rf'MASTER_IMG_FLAT_{cls.target()}_{cls.band()}'

    @classmethod
    def title(cls):
        return f"{cls.band()} {cls.get_target_string()} master flat"

    @classmethod
    def description(cls):
        return f"Master flat frame for {cls.band()} data"
