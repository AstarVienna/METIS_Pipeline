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
from pymetis.classes.dataitems import Raw
from pymetis.classes.mixins import (BandSpecificMixin, BandLmMixin, BandNMixin,
                                    TargetSpecificMixin, TargetStdMixin, TargetSciMixin)


class ImageRaw(BandSpecificMixin, TargetSpecificMixin, Raw, abstract=True):
    """
    Abstract intermediate class for image raws.
    """

    _oca_keywords = {"DPR.CATG", "DPR.TECH", "DPR.TYPE", "INS.OPTI3.NAME",
                     "INS.OPTI9.NAME", "INS.OPTI10.NAME", "DRS.FILTER"}

    @classmethod
    def name(cls) -> str:
        return rf'{cls.band()}_IMAGE_{cls.target()}_RAW'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.band()} {cls.get_target_string()} raw'

    @classmethod
    def description(cls) -> str:
        return rf"Raw exposure of a {cls.get_target_string()} in the {cls.band()} image mode."


class LmImageStdRaw(BandLmMixin, TargetStdMixin, ImageRaw):
    pass


class LmImageSciRaw(BandLmMixin, TargetSciMixin, ImageRaw):
    pass


class NImageStdRaw(BandNMixin, TargetStdMixin, ImageRaw):
    pass


class NImageSciRaw(BandNMixin, TargetSciMixin, ImageRaw):
    pass
