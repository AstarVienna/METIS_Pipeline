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

from pathlib import Path
from typing import Union, TypeVar, Generic, Optional

import cpl
from cpl.core import (Image as CplImage,
                      Table as CplTable,
                      PropertyList as CplPropertyList)


class Hdu:
    """
    A loose association of a header and data
    """
    def __init__(self,
                 header: CplPropertyList,
                 data: Optional[CplImage | CplTable],
                 *,
                 klass: type[CplImage | CplTable],
                 extno: Optional[int] = 0,
                ) -> None:
        """

        Parameters
        ----------
        header: CplPropertyList
        data: CplImage | CplTable
            Data inside this HDU. Might be empty.
        extno:
            Can be used to access data by index.
        """
        self.header = header
        self.data = data
        self.klass = klass
        self.extno = extno

        try:
            self.name = self.header['EXTNAME'].value
        except KeyError:
            self.name = 'PRIMARY'


    def __repr__(self) -> str:
        return f"<HDU {self.name=}, {self.extno=}>"

class MetisImage:
    """
    A 2D data array, complete with uncertainty and data quality map.
    Contains methods for saving loading and what not.
    """
    def __init__(self,
                 data: CplImage,
                 uncertainty: CplImage,
                 quality: CplImage):
        self.data = data
        self.uncertainty = uncertainty
        self.quality = quality

    def save(self,
             filename: Path):
        self.data.save(filename)