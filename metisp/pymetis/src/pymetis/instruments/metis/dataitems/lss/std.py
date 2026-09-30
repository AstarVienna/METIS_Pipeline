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

from pymetis.engine.dataitems import TableDataItem
from pymetis.instruments.metis.mixins import BandLmMixin, BandNMixin
from pymetis.instruments.metis import keywords as kw


class LssStd1d(TableDataItem, abstract=True):
    _name_template = r'{band}_LSS_STD_1D'
    _title_template = "{band} LSS 1D standard star spectrum"
    _description_template = "Extracted {band} 1D standard star spectrum."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = frozenset({kw.PRO_CATG, kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME, kw.DRS_SLIT})


class LmLssStd1d(BandLmMixin, LssStd1d):
    pass


class NLssStd1d(BandNMixin, LssStd1d):
    pass


class RefStdCat(TableDataItem):
    _name_template = r'REF_STD_CAT'
    _title_template = "ref standard catalogue"
    _description_template = "Catalogue with spectra of standard reference stars"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _static = True
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({kw.PRO_CATG})


class AoPsfModel(TableDataItem):
    _name_template = r'AO_PSF_MODEL'
    _title_template = "AO PSD model"
    _description_template = "Model of the AO induced PSF."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({kw.PRO_CATG})
