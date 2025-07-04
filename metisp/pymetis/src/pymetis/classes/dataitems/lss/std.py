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

from pymetis.classes.dataitems import TableDataItem, parametrize
from pymetis.classes.mixins import BandSpecificMixin


@parametrize("{band}LssStd1d", band=['LM', 'N'])
class LssStd1d(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template = r'{band}_LSS_STD_1D'
    _title_template = "{band} LSS 1D standard star spectrum"
    _description_template = "Extracted {band} 1D standard star spectrum."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _oca_keywords = {'PRO.CATG', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME', 'DRS.SLIT'}
