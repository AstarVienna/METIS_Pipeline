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

from pymetis.classes.dataitems import parametrize
from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class Linearity(DataItem, abstract=True):
    _frame_group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG'}

    @classmethod
    def name(cls):
        return rf'LINEARITY_{cls.detector()}'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.detector()} linearity'

    @classmethod
    def description(cls):
        return rf"Coefficients for the pixel {cls.detector()} non-linearity correction"



class Linearity2rg(Detector2rgMixin, Linearity):
    pass


class LinearityGeo(DetectorGeoMixin, Linearity):
    pass


class LinearityIfu(DetectorIfuMixin, Linearity):
    pass
