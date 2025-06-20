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
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, TargetSciMixin, TargetStdMixin
from pymetis.classes.mixins.band import BandNMixin, BandLmMixin, BandSpecificMixin, BandIfuMixin
from pymetis.classes.mixins.target import TargetSpecificMixin


class BackgroundReduced(DataItem, ABC):
    _title = "background-reduced"
    _detector = None
    _description = None
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    @classmethod
    def _pro_catg(cls):
        return rf"{cls.detector()}_BACKGROUND_REDUCED"


class BasicReduced(DataItem, ABC):
    @classmethod
    def name(cls):
        return rf"{cls.band()}_SCI_BASIC_REDUCED"


class StdBasicReduced(Detector2rgMixin, BackgroundReduced):
    _description = "Standard detrended exposure of the LM image mode."
    _tag = rf"LM_STD_BASIC_REDUCED"


class SciBasicReducedGeo(DetectorGeoMixin, BackgroundReduced):
    _description = "Science grade detrended exposure of the LM image mode."


class SkyBasicReduced(DataItem, ABC):
    _title = "sky basic-reduced exposure"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Detrended exposure of the sky."
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe


class BackgroundSubtracted(BandSpecificMixin, TargetSpecificMixin, DataItem, ABC):
    _title = "background-subtracted"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe

    @classmethod
    def name(cls):
        return rf"{cls.band()}_{cls.target()}_BKG_SUBTRACTED"

    @classmethod
    def description(cls):
        return rf"Thermal background subtracted images of science {cls.band()} exposures."


class LmStdBackgroundSubtracted(BandLmMixin, TargetStdMixin, BackgroundSubtracted):
    pass


class LmSciBackgroundSubtracted(BandLmMixin, TargetSciMixin, BackgroundSubtracted):
    pass


class NStdBackgroundSubtracted(BandNMixin, TargetStdMixin, BackgroundSubtracted):
    pass


class NSciBackgroundSubtracted(BandNMixin, TargetSciMixin, BackgroundReduced):
    pass