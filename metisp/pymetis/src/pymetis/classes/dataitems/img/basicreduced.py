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
    TargetSpecificMixin, BandLmMixin, TargetSciMixin, TargetStdMixin, TargetSkyMixin, BandNMixin


class BasicReduced(BandLmMixin, TargetSpecificMixin, DataItem, abstract=True):
    _title: str = "basic reduced"
    _description: str = None
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    @classmethod
    def name(cls) -> str:
        return rf'{cls.band()}_{cls.target()}_BASIC_REDUCED'

    @classmethod
    def title(cls):
        return f"{cls.band()} {cls.get_target_string()} basic reduced"

    @classmethod
    def _pro_catg(cls):
        return rf"{cls._band}_BASIC_REDUCED"

    @classmethod
    def description(cls):
        return f"Detrended exposure of the {cls.band():s} image mode."


class LmStdBasicReduced(TargetStdMixin, BasicReduced):
    pass


class LmSciBasicReduced(TargetSkyMixin, BasicReduced):
    pass


class LmSkyBasicReduced(TargetSciMixin, BasicReduced):
    @classmethod
    def description(cls):
        return "Detrended exposure of the sky."


class Calibrated(BandSpecificMixin, TargetSpecificMixin, DataItem, abstract=True):
    @classmethod
    def name(cls):
        return rf'{cls.band()}_{cls.target()}_CALIBRATED'

    @classmethod
    def description(cls):
        return f"calibrated {cls.band()} {cls.get_target_string()}"


class LmStdCalibrated(BandLmMixin, TargetStdMixin, Calibrated):
    pass


class LmSciCalibrated(BandLmMixin, TargetSciMixin, Calibrated):
    pass


class NStdCalibrated(BandNMixin, TargetStdMixin, Calibrated):
    pass


class NSciCalibrated(BandNMixin, TargetSciMixin, Calibrated):
    pass