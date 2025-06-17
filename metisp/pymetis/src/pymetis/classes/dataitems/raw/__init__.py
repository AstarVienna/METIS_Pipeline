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
from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.mixins.band import BandLmMixin, BandIfuMixin, BandNMixin
from pymetis.classes.mixins.target import TargetSpecificMixin


class Raw(DataItem, ABC):
    """
    Abstract intermediate class for all raw items.
    """
    _group = cpl.ui.Frame.FrameGroup.RAW
    _description = "Abstract base class for all raw inputs. Please subclass."


class ImageRaw(TargetSpecificMixin, Raw, ABC):
    """
    Abstract intermediate class for image raws.
    """

    _oca_keywords = {"DPR.CATG", "DPR.TECH", "DPR.TYPE", "INS.OPTI3.NAME",
                     "INS.OPTI9.NAME", "INS.OPTI10.NAME", "DRS.FILTER"}

    @classmethod
    def name(cls) -> str:
        return rf'{cls.band():s}_IMAGE_{cls.target():s}_RAW'

    @classmethod
    def title(cls) -> str:
        return rf'{cls.band():s} {cls.get_target_string():s} raw'

    @classmethod
    def description(cls) -> str:
        return rf"Raw exposure of a {cls.get_target_string():s} in the {cls.band():s} image mode."


class LmImageStdRaw(BandLmMixin, TargetStdMixin, ImageRaw):
    pass


class LmImageSciRaw(BandLmMixin, TargetSciMixin, ImageRaw):
    pass


class NImageStdRaw(BandNMixin, TargetStdMixin, ImageRaw):
    pass


class NImageSciRaw(BandNMixin, TargetSciMixin, ImageRaw):
    pass


class IfuSciRaw(BandIfuMixin, TargetSciMixin, ImageRaw):
    _title = r"IFU science raw"
    _oca_keywords = {"DPR.CATG", "DPR.TECH", "DPR.TYPE", "INS.OPTI3.NAME",
                     "INS.OPTI9.NAME", "INS.OPTI10.NAME", "INS.OPTI11.NAME",
                     "DRS.IFU"}