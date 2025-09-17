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
from cpl.core import Msg, PropertyList as CplPropertyList

from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class TableDataItem(DataItem, abstract=True):
    _frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.TABLE

    def __init__(self,
                 primary_header: cpl.core.PropertyList,
                 *hdus: cpl.core.Table):
        super().__init__(primary_header, *hdus)

    def save_extensions(self,
                        filename: str) -> None:
        for hdu in self.hdus:
            hdu.save(self.header, self.header, filename, cpl.core.io.EXTEND)