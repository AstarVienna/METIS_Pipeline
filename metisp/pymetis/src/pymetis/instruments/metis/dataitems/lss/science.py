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
from cpl.core import Table, Image

from pymetis.engine.dataitems import ImageDataItem, TableDataItem
from pymetis.instruments.metis.mixins import BandLmMixin, BandNMixin, TargetSciMixin, TargetStdMixin
from pymetis.instruments.metis import keywords as kw


class LssObjMap(ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_{target}_OBJ_MAP'
    _title_template = "{band} LSS {target} object map"
    _description_template = "Pixel map of object pixels (QC)"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = frozenset({kw.PRO_CATG, kw.DRS_SLIT})

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
    }


class LmLssStdObjMap(BandLmMixin, TargetStdMixin, LssObjMap):
    pass


class NLssStdObjMap(BandNMixin, TargetStdMixin, LssObjMap):
    pass


class LmLssSciObjMap(BandLmMixin, TargetSciMixin, LssObjMap):
    pass


class NLssSciObjMap(BandNMixin, TargetSciMixin, LssObjMap):
    pass


class LssSkyMap(ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_{target}_SKY_MAP'
    _title_template = "{band} LSS {target} sky map"
    _description_template = "Image with detected plain sky pixels of the {target} observation."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
    }


class LmLssStdSkyMap(BandLmMixin, TargetStdMixin, LssSkyMap):
    pass


class NLssStdSkyMap(BandNMixin, TargetStdMixin, LssSkyMap):
    pass


class LmLssSciSkyMap(BandLmMixin, TargetSciMixin, LssSkyMap):
    pass


class NLssSciSkyMap(BandNMixin, TargetSciMixin, LssSkyMap):
    pass


class LssSci1d(TableDataItem, abstract=True):
    _name_template = r'{band}_LSS_SCI_1D'
    _title_template = "{band} LSS 1D science spectrum"
    _description_template = "Extracted {band} 1D science spectrum."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})


class LmLssSci1d(BandLmMixin, LssSci1d):
    pass


class NLssSci1d(BandNMixin, LssSci1d):
    pass


class LssSci2d(ImageDataItem, abstract=True):
    _name_template = r'{band}_LSS_SCI_2D'
    _title_template = "{band} LSS 2D science spectrum"
    _description_template = "Rectified 2D {band} spectrum of science object."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
    }


class LmLssSci2d(BandLmMixin, LssSci2d):
    pass


class NLssSci2d(BandNMixin, LssSci2d):
    pass


class LssSciFlux1d(TableDataItem, abstract=True):
    """
    Final flux calibrated 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_1D'
    _title_template = "{band} LSS SCI 1D flux"
    _description_template = "Extracted, flux-calibrated 1D science spectrum"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})

    _schema = {
        'PRIMARY': None,
        'TABLE': Table,
    }


class LmLssSciFlux1d(BandLmMixin, LssSciFlux1d):
    pass


class NLssSciFlux1d(BandNMixin, LssSciFlux1d):
    pass


class LssSciFlux2d(ImageDataItem, abstract=True):
    """
    Final flux calibrated 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_2D'
    _title_template = "{band} LSS SCI 2D flux"
    _description_template = "Rectified, flux-calibrated 2D {band}-band spectrum of the science object."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
    }


class LmLssSciFlux2d(BandLmMixin, LssSciFlux2d):
    pass


class NLssSciFlux2d(BandNMixin, LssSciFlux2d):
    pass


class LssSciFluxTellCorr1d(TableDataItem, abstract=True):
    """
    Final flux calibrated, telluric corrected 1D spectrum of standard star
    """
    _name_template = r'{band}_LSS_SCI_FLUX_TELLCORR_1D'
    _title_template = "{band} LSS science flux-calibrated telluric-corrected"
    _description_template = "Extracted, flux-calibrated, telluric-corrected 1D science spectrum"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI12_NAME, kw.INS_OPTI13_NAME, kw.INS_OPTI14_NAME, kw.DRS_SLIT})


class LmLssSciFluxTellCorr1d(BandLmMixin, LssSciFluxTellCorr1d):
    pass


class NLssSciFluxTellCorr1d(BandNMixin, LssSciFluxTellCorr1d):
    pass

