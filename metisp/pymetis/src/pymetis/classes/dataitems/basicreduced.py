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
    TargetSpecificMixin, BandLmMixin, TargetSciMixin


class BasicReduced(DataItem, abstract=True):
    _title: str = "basic reduced"
    _band: str = 'LM'
    _description: str = None
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    @classmethod
    def _pro_catg(cls):
        return rf"{cls._band}_BASIC_REDUCED"

    @classmethod
    def description(cls):
        return f"Detrended exposure of the LM image mode."


class LmStdBasicReduced(Detector2rgMixin, BasicReduced):
    _tag: str = r"LM_STD_BASIC_REDUCED"
    _target: str = 'STD'


class LmSciBasicReduced(DetectorGeoMixin, BasicReduced):
    _tag: str = r"LM_SCI_BASIC_REDUCED"
    _target: str = 'SCI'


class SkyBasicReduced(DataItem, ABC):
    _name = r'LM_SKY_BASIC_REDUCED'
    _title = "Sky basic-reduced exposure"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Detrended exposure of the sky."
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe



class Calibrated(BandSpecificMixin, TargetSpecificMixin, DataItem, ABC):
    @classmethod
    def name(cls):
        return rf'{cls.band()}_{cls.target()}_CALIBRATED'

    @classmethod
    def description(cls):
        return


class LmSciCalibrated(BandLmMixin, TargetSciMixin, Calibrated):
    pass