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
from pymetis.classes.mixins.band import BandNMixin, BandLmMixin
from pymetis.classes.mixins.detector import DetectorSpecificMixin, Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.classes.mixins.target import TargetLampMixin, TargetTwilightMixin, TargetSpecificMixin


class FlatRaw(DetectorSpecificMixin, TargetSpecificMixin, Raw):
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                     'INS.OPTI3.NAME', 'INS.OPTI12.NAME', 'INS.OPTI13.NAME', 'DRS.FILTER'}

    @classmethod
    def name(cls):
        return rf'{cls.band()}_FLAT_{cls.target()}_RAW'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.band()} flat {cls.get_target_string():s} raw'


class LmFlatLampRaw(BandLmMixin, TargetLampMixin, FlatRaw):
    pass


class LmFlatTwilightRaw(BandLmMixin, TargetTwilightMixin, FlatRaw):
    pass


class NFlatLampRaw(BandNMixin, TargetLampMixin, FlatRaw):
    pass


class NFlatTwilightRaw(BandNMixin, TargetTwilightMixin, FlatRaw):
    pass

