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
from pymetis.classes.dataitems.raw import Raw
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, DetectorSpecificMixin
from pymetis.classes.mixins.band import BandSpecificMixin, BandLmMixin, BandNMixin, BandIfuMixin


class RsrfIfu(DetectorIfuMixin, DataItem):
    _name = r'RSRF_IFU'
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "2D relative spectral response function"
    _title = "RSRF IFU"


class RsrfRaw(DetectorSpecificMixin, BandSpecificMixin, Raw, abstract=True):
    @classmethod
    def name(cls):
        return rf'{cls.band()}_LSS_RSRF_RAW'


class LmLssRsrfRaw(Detector2rgMixin, BandLmMixin, RsrfRaw):
    pass


class NLssRsrfRaw(DetectorGeoMixin, BandNMixin, RsrfRaw):
    pass


class IfuRsrfRaw(DetectorIfuMixin, BandIfuMixin, RsrfRaw):
    _title = "IFU RSRF raw image"

    @classmethod
    def name(cls):
        """
        Here the name is overridden as it is not LSS.
        """
        return r'IFU_RSRF_RAW'


class IfuRsrfBackground(DetectorIfuMixin, DataItem):
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _name = r'IFU_RSRF_BACKGROUND'
