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
from typing import Literal

import cpl

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, TargetSciMixin
from pymetis.classes.mixins.band import BandNMixin, BandLmMixin


class BackgroundReduced(DataItem, ABC):
    _title: str = "background-reduced"
    _detector: str = None
    _description: str = None
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    @classmethod
    def _pro_catg(cls):
        return rf"{cls._detector}_DISTORTION_TABLE"


class StdBasicReduced(Detector2rgMixin, BackgroundReduced):
    _description: str = "Standard detrended exposure of the LM image mode."
    _tag: str = r"LM_STD_BASIC_REDUCED"


class SciBasicReducedGeo(DetectorGeoMixin, BackgroundReduced):
    _description: str = "Science grade detrended exposure of the LM image mode."
    _tag: str = r"LM_SCI_BASIC_REDUCED"


class SkyBasicReduced(DataItem, ABC):
    _title = "sky basic-reduced exposure"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Detrended exposure of the sky."
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe



class BackgroundSubtracted(DataItem, ABC):
    _title = "background-subtracted"
    _band: str = r'LM'
    _detector: str = r'2RG'
    _target: Literal['SCI', 'STD'] = None
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe

    @classmethod
    def name(cls):
        return rf"{cls._band}_{cls._target}_BKG_SUBTRACTED"

    @classmethod
    def description(cls):
        return rf"Thermal background subtracted images of science {cls._band} exposures."


class LmSciBackgroundSubtracted(BandLmMixin, TargetSciMixin, BackgroundSubtracted):
    pass


class NSciBackgroundSubtracted(BandNMixin, TargetSciMixin, BackgroundReduced):
    pass