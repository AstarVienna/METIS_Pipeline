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

from .dataitem import DataItem
from ..mixins import Detector2rgMixin, DetectorIfuMixin


class BadPixMap(DataItem, ABC):
    _title: str = "bad pixel map"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Bad pixel map. Also contains detector masks."
    _oca_keywords: set[str] = {'PRO.CATG'}
    _detector: str = None

    @classmethod
    def _pro_catg(cls):
        return rf"BADPIX_MAP_{cls._detector}"


class BadPixMap2rg(Detector2rgMixin, BadPixMap):
    pass


class BadPixMapGeo(Detector2rgMixin, BadPixMap):
    pass


class BadPixMapIfu(DetectorIfuMixin, BadPixMap):
    pass