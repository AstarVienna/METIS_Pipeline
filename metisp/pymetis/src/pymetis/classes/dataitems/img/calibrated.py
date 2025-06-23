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

from pymetis.classes.dataitems import DataItem
from pymetis.classes.mixins import BandSpecificMixin, BandLmMixin, BandNMixin


class ScienceCalibrated(BandSpecificMixin, DataItem, abstract=True):
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    @classmethod
    def name(cls):
        return rf'{cls.band()}_SCI_CALIBRATED'

    @classmethod
    def title(cls):
        return f"{cls.band()} science calibrated"


class LmScienceCalibrated(BandLmMixin, ScienceCalibrated,
                          description="LM band image with flux calibration, WC coordinate system"
                                      "and distortion information"):
    pass

class NScienceCalibrated(BandNMixin, ScienceCalibrated,
                         description="N band image with flux calibration and distortion information"):
    pass