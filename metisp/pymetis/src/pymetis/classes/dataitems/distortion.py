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
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.classes.mixins.band import BandLmMixin, BandNMixin, BandIfuMixin


class DistortionTable(DataItem, ABC):
    _title: str = "distortion table"
    _detector: str = None
    _description: str = "Table of distortion coefficients"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}

    @classmethod
    def _pro_catg(cls):
        return rf"{cls._detector}_DISTORTION_TABLE"


class DistortionTable2rg(Detector2rgMixin, DistortionTable):
    _description: str = "Table of distortion coefficients for a 2RG data set"
    pass


class DistortionTableGeo(DetectorGeoMixin, DistortionTable):
    _description: str = "Table of distortion coefficients for a GEO data set"
    pass


class DistortionTableIfu(DetectorIfuMixin, DistortionTable):
    _description: str = "Table of distortion coefficients for an IFU data set"
    pass


class DistortionRaw(DataItem, ABC):
    _title: str = "distortion raw"
    _name: str = None
    _description: str = "Raw data for dark subtraction in other recipes."
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

    @classmethod
    def description(cls):
        return f"Raw data for dark subtraction in other recipes in the {cls.band()} band."


class LmDistortionRaw(BandLmMixin, DistortionRaw):
    pass

class NDistortionRaw(BandNMixin, DistortionRaw):
    pass

class IfuDistortionRaw(BandIfuMixin, DistortionRaw):
    pass