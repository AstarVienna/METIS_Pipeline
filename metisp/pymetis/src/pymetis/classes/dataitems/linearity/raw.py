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

from pymetis.classes.dataitems.raw import Raw
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class LinearityRaw(Raw, abstract=True):
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE'}

    @classmethod
    def name(cls):
        return rf'DETLIN_{cls.detector()}_RAW'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.detector()} linearity raw'

    @classmethod
    def description(cls):
        return rf"Raw data for non-linearity determination for {cls.detector()} observations"


class Linearity2rgRaw(Detector2rgMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.FILTER'}


class LinearityGeoRaw(DetectorGeoMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.FILTER'}


class LinearityIfuRaw(DetectorIfuMixin, LinearityRaw):
    _oca_keywords = LinearityRaw._oca_keywords | {'DRS.IFU'}