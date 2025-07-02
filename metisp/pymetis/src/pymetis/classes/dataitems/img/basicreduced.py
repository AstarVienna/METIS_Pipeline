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

from pymetis.classes.dataitems.dataitem import ImageDataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, BandSpecificMixin, \
    TargetSpecificMixin, BandLmMixin, TargetSciMixin, TargetStdMixin, TargetSkyMixin, BandNMixin


class BasicReduced(BandLmMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_{target}_BASIC_REDUCED'
    _title_template = "{band} {target} basic reduced"
    _description_template = "Detrended exposure of the {band} image mode."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}


class LmStdBasicReduced(TargetStdMixin, BasicReduced):
    pass


class LmSciBasicReduced(TargetSkyMixin, BasicReduced):
    pass


class LmSkyBasicReduced(TargetSciMixin, BasicReduced):
    @classmethod
    def description(cls):
        return "Detrended exposure of the sky."


class Calibrated(BandSpecificMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_{target}_CALIBRATED'
    _title_template = '{band} {target} calibrated'
    _description_template = 'Calibrated {band} {target}'
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.RAW  # This actually has to be raw as it is "primary input" (rite-of-passage)
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}


class LmStdCalibrated(BandLmMixin, TargetStdMixin, Calibrated):
    pass


class LmSciCalibrated(BandLmMixin, TargetSciMixin, Calibrated):
    _description_template = "LM band image with flux calibration, WC coordinate system and distortion information"


class NStdCalibrated(BandNMixin, TargetStdMixin, Calibrated):
    pass


class NSciCalibrated(BandNMixin, TargetSciMixin, Calibrated):
    _description_template = "N band image with flux calibration and distortion information"


class NSciRestored(BandNMixin, ImageDataItem):
    _name_template = r'N_SCI_RESTORED'
    _description_template = "N band image with a single positive beam restored from chop-nod image"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}
