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

from pymetis.classes.dataitems import ImageDataItem, TableDataItem, parametrize
from pymetis.classes.mixins import BandSpecificMixin, TargetSpecificMixin, BandLmMixin, BandNMixin, TargetSciMixin, \
    TargetStdMixin


@parametrize('{band}Lss{target}ObjMap', band=['LM', 'N'], target=['STD', 'SCI'])
class LssObjMap(BandSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_{target}_OBJ_MAP'
    _title_template = "{band} LSS {target} object map"
    _description_template = "Pixel map of object pixels (QC)"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


@parametrize('{band}Lss{target}SkyMap', band=['LM', 'N'], target=['STD', 'SCI'])
class LssSkyMap(BandSpecificMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_{target}_SKY_MAP'
    _title_template = "{band} LSS {target} sky map"
    _description_template = "Image with detected plain sky pixels of the {target} observation."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}


class LssSci1d(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template = r'{band}_LSS_SCI_1D'
    _title_template = "{band} LSS 1D science spectrum"
    _description_template = "Extracted {band} 1D science spectrum."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}


class LmLssSci1d(BandLmMixin, LssSci1d):
    pass


class NLssSci1d(BandNMixin, LssSci1d):
    pass



@parametrize("{band}LssSci2d", band=['LM', 'N'])
class LssSci2d(BandSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_SCI_2D'
    _title_template = "{band} LSS 2D science spectrum"
    _description_template = "Rectified 2D {band} spectrum of science object."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}


class LssSciFlux1d(BandSpecificMixin, TableDataItem, abstract=True):
    """
    Final flux calibrated 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_1D'
    _title_template = "{band} LSS SCI 1D flux"
    _description_template = "Extracted, flux-calibrated 1D science spectrum"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}


class LmLssSciFlux1d(BandLmMixin, LssSciFlux1d):
    pass


class NLssSciFlux1d(BandNMixin, LssSciFlux1d):
    pass


@parametrize("{band}LssSciFlux2d", band=['LM', 'N'])
class LssSciFlux2d(BandSpecificMixin, ImageDataItem, abstract=True):
    """
    Final flux calibrated 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_2D'
    _title_template = "{band} LSS SCI 2D flux"
    _description_template = "Rectified, flux-calibrated 2D {band}-band spectrum of the science object."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}


@parametrize("{band}LssSciFluxTellCorr1d", band=['LM', 'N'])
class LssSciFluxTellCorr1d(BandSpecificMixin, TableDataItem, abstract=True):
    """
    Final flux calibrated, telluric corrected 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_TELL_1D'
    _title_template = "{band} LSS science flux-calibrated telluric-corrected"
    _description_template = "Extracted, flux-calibrated, telluric-corrected 1D science spectrum"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI12.NAME', 'INS.OPTI13.NAME', 'INS.OPTI14.NAME', 'DRS.SLIT'}

