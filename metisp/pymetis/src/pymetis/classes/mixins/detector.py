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


class DetectorSpecificMixin(ABC):
    _detector: str = None

    @classmethod
    def detector(cls) -> str:
        return cls._detector or r'{detector}'

    def __init_subclass__(cls, *, detector=None, **kwargs):
        cls._detector = detector
        super().__init_subclass__(**kwargs)


class Detector2rgMixin(DetectorSpecificMixin, detector='2RG'):
    pass


class DetectorGeoMixin(DetectorSpecificMixin, detector='GEO'):
    pass


class DetectorIfuMixin(DetectorSpecificMixin, detector='IFU'):
    pass
