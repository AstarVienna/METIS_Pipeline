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
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}



class IfuReduced(IfuBase, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_REDUCED'

    @classmethod
    def title(cls):
        return f"IFU {cls.get_target_string():s} reduced"

    @classmethod
    def description(cls) -> str:
        return f"Reduced 2D detector image of a {cls.get_target_string()}"


class IfuStdReduced(TargetStdMixin, IfuReduced):
    pass


class IfuSciReduced(TargetSciMixin, IfuReduced):
    pass


class IfuReducedCube(IfuBase, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_REDUCED_CUBE'


class IfuStdReducedCube(TargetStdMixin, IfuReducedCube):
    _description = "Reduced 2D detector image of spectroscopic flux standard star."


class IfuSciReducedCube(TargetSciMixin, IfuReducedCube):
    _description = "A rectified spectral cube with a linear wavelength grid."


class IfuReduced1d(IfuBase, abstract=True):
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_REDUCED_1D'


class IfuStdReduced1d(TargetStdMixin, IfuReduced1d):
    _description = "Spectrum of a reduced telluric standard star."


class IfuSciReduced1d(TargetSciMixin, IfuReduced1d):
    _description = "Spectrum of a science object."


class IfuCombined(IfuBase, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_COMBINED'


class IfuStdCombined(TargetStdMixin, IfuCombined):
    _description = "Spectral cube of a standard star, combining multiple exposures."


class IfuSciCombined(TargetSciMixin, IfuCombined):
    _description = "Spectral cube of science object, combining multiple exposures."
