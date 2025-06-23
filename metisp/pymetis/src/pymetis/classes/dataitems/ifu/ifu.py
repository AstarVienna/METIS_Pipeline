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

import cpl.ui

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.mixins.band import BandIfuMixin
from pymetis.classes.mixins.target import TargetSpecificMixin


class IfuBase(TargetSpecificMixin, BandIfuMixin, DataItem, abstract=True):
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class IfuBackground(IfuBase, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_BACKGROUND'

class IfuStdBackground(TargetStdMixin, IfuBackground):
    _description = "Reduced 2D detector image of background."


class IfuSciBackground(TargetSciMixin, IfuBackground):
    _description = "Reduced 2D detector image of background."


class IfuReduced(IfuBase, abstract=True):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB

    @classmethod
    def title(cls):
        return f"IFU {cls.get_target_string():s} reduced"

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_REDUCED'


class IfuStdReduced(TargetStdMixin, IfuReduced):
    _description = "Reduced 2D detector image of a standard star."


class IfuSciReduced(TargetSciMixin, IfuReduced):
    _description = "Reduced 2D detector image of a science object."


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
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_REDUCED_1D'


class IfuStdReduced1d(TargetStdMixin, IfuReduced1d):
    _description = "Spectrum of a reduced telluric standard star."


class IfuSciReduced1d(TargetSciMixin, IfuReduced1d):
    _description = "Spectrum of a science object."


class IfuCombined(IfuBase, ABC):
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_type = cpl.ui.Frame.FrameType.IMAGE

    @classmethod
    def name(cls):
        return rf'IFU_{cls.target():s}_COMBINED'


class IfuStdCombined(TargetStdMixin, IfuCombined):
    _description = "Spectral cube of a standard star, combining multiple exposures."


class IfuSciCombined(TargetSciMixin, IfuCombined):
    _description = "Spectral cube of science object, combining multiple exposures."
