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

from pymetis.classes.dataitems import ImageDataItem, parametrize
from pymetis.classes.mixins import BandSpecificMixin


@parametrize(r'{band}LssCurve', band=['LM', 'N'])
class LssCurve(BandSpecificMixin, ImageDataItem, abstract=True):
    """
    Trace curvature
    """
    _name_template = r"{band}_LSS_CURVE"
    _title_template = "{band} LSS curvature"
    _description_template = "Trace curvature information"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


@parametrize(r'{band}DistSol', band=['LM', 'N'])
class LssDistSol(BandSpecificMixin, ImageDataItem, abstract=True):
    """
    Distortion solution
    """
    _name_template = r'{band}_LSS_DIST_SOL'
    _title_template = "{band} distortion solution"
    _description_template = "Distortion solution for rectifying"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


@parametrize(r'{band}WaveGuess', band=['LM', 'N'])
class LssWaveGuess(BandSpecificMixin, ImageDataItem, abstract=True):
    """
    First guess of the wavelength solution
    """
    _name_template = r'{band}_LSS_WAVE_GUESS'
    _title_template = "{band} LSS wavelength solution guess"
    _description_template = "First guess of the wavelength solution"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}