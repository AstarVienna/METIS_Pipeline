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
from typing import Union, TypeVar, Generic

from cpl.core import (Image as CplImage,
                      Table as CplTable,
                      PropertyList as CplPropertyList)

T = TypeVar('T')

class Hdu(Generic[T]):
    """
    A loose association of a header and data
    """
    def __init__(self,
                 header: CplPropertyList,
                 data: T):
        self.header = header
        self.data = data


class ImageHdu(Hdu[CplImage]):
    def __init__(self,
                 header: CplPropertyList,
                 image: CplImage):
        super().__init__(header, image)


class TableHdu(Hdu[CplTable]):
    def __init__(self,
                 header: CplPropertyList,
                 table: CplTable):
        super().__init__(header, table)


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