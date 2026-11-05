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
from cpl.core import Image

from pymetis.engine.dataitems import TableDataItem, ImageDataItem
from pymetis.instruments.metis.mixins import BandLmMixin, BandNMixin


class LssTrace(TableDataItem, abstract=True):
    """
    Final trace table
    """
    _name_template = r'{band}_LSS_TRACE'
    _title_template = r"{band} LSS trace"
    _description_template = "Table with polynomials describing the location of the traces on the detector"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'DRS.SLIT'})


class LmLssTrace(BandLmMixin, LssTrace):
    pass


class NLssTrace(BandNMixin, LssTrace):
    pass

class LssTraceImage(ImageDataItem, abstract=True):
    """
    Master trace image
    """
    _name_template = r'MASTER_LSS_TRACE_{band}'
    _title_template = r"Master LSS trace {band}"
    _description_template = "Master trace"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'DRS.SLIT'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class LmLssTraceImage(BandLmMixin, LssTraceImage):
    pass


class NLssTraceImage(BandNMixin, LssTraceImage):
    pass
