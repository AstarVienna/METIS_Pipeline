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

from pymetis.classes.dataitems import ImageDataItem
from pymetis.classes.mixins import DetectorIfuMixin
from cpl.core import Table, Image


class IfuWavecalRaw(DetectorIfuMixin, ImageDataItem):
    _name_template = r'IFU_WAVE_RAW'
    _title_template = 'IFU wavecal raw'
    _description_template = ("Raw exposure of the WCU laser sources through the IFU to"
                             "achieve the first guess of the wavelength calibration.")
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.NONE
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                     'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME',
                     'DRS.IFU'}


class IfuWavecal(DetectorIfuMixin, ImageDataItem):
    _name_template = r'IFU_WAVECAL'
    _title_template = "IFU wave calibration"
    _description_template = "Image with wavelength at each pixel."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

    _schema = {
        'PRIMARY': None,
    } | {
        fr'DET{det:1d}': Image for det in range(1, 5)
    }
