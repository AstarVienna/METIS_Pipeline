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
from pymetis.classes.mixins.detector import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class DarkRaw(DetectorSpecificMixin, Raw, abstract=True):
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE', 'DET.ID', 'DET.DIT'}

    @classmethod
    def name(cls):
        return rf'DARK_{cls.detector()}_RAW'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.detector()} dark raw'

    @classmethod
    def description(cls) -> str:
        return f"Raw data for creating a {cls.detector()} master dark."


class Dark2rgRaw(Detector2rgMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.FILTER'}


class DarkGeoRaw(DetectorGeoMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.FILTER'}


class DarkIfuRaw(DetectorIfuMixin, DarkRaw):
    _oca_keywords = DarkRaw._oca_keywords | {'DRS.IFU'}