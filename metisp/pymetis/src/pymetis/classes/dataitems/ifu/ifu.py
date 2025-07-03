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

import cpl.ui

from pymetis.classes.dataitems.dataitem import ImageDataItem
from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.mixins.band import BandIfuMixin
from pymetis.classes.mixins.target import TargetSpecificMixin


class IfuBase(TargetSpecificMixin, BandIfuMixin, ImageDataItem, abstract=True):
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class IfuReduced(IfuBase, abstract=True):
    _name_template = r'IFU_{target}_REDUCED'
    _title_template = "IFU {target} reduced"
    _description_template = "Reduced 2D detector image of a {target}"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB


class IfuStdReduced(TargetStdMixin, IfuReduced):
    pass


class IfuSciReduced(TargetSciMixin, IfuReduced):
    pass


class IfuReducedCube(IfuBase, abstract=True):
    _name_template = r'IFU_{target}_REDUCED_CUBE'
    _title_template = "IFU {target} reduced cube"
    _description_template = "Reduced cube"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT


class IfuStdReducedCube(TargetStdMixin, IfuReducedCube):
    _description_template = "Reduced 2D detector image of spectroscopic flux standard star."


class IfuSciReducedCube(TargetSciMixin, IfuReducedCube):
    _description_template = "A rectified spectral cube with a linear wavelength grid."


class IfuReduced1d(IfuBase, abstract=True):
    _name_template = r'IFU_{target}_REDUCED_1D'
    _title_template = r'IFU {target} reduced 1D spectrum'
    _description_template = "Reduced 1D spectrum"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE


class IfuStdReduced1d(TargetStdMixin, IfuReduced1d):
    _description_template = "Spectrum of a reduced telluric standard star."


class IfuSciReduced1d(TargetSciMixin, IfuReduced1d):
    _description_template = "Spectrum of a science object."


class IfuCombined(IfuBase, abstract=True):
    _name_template = r'IFU_{target}_COMBINED'
    _title_template = "spectral cube of science object"
    _description_template = "Spectral cube of a standard star, combining multiple exposures."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT


class IfuStdCombined(TargetStdMixin, IfuCombined):
    _description_template = "Spectral cube of a standard star, combining multiple exposures."


class IfuSciCombined(TargetSciMixin, IfuCombined):
    _description_template = "Spectral cube of science object, combining multiple exposures."


class IfuScienceCubeCalibrated(BandIfuMixin, ImageDataItem):
    _name_template = r'IFU_SCI_CUBE_CALIBRATED'
    _title_template = "IFU science cube calibrated"
    _description_template = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}
