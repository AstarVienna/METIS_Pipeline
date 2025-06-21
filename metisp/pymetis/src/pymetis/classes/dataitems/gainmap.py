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
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class GainMap(DataItem, abstract=True):
    _title: str = "gain map"
    _detector: str = None
    _description: str = "Gain map"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords: set[str] = {'PRO.CATG'}

    @classmethod
    def name(cls):
        return rf"GAIN_MAP_{cls.detector()}"

    @classmethod
    def pro_catg(cls):
        return rf"GAIN_MAP_{cls.detector()}"


class GainMap2rg(Detector2rgMixin, GainMap):
    pass


class GainMapGeo(DetectorGeoMixin, GainMap):
    pass


class GainMapIfu(DetectorIfuMixin, GainMap):
    pass
