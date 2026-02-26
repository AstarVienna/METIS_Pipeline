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
                      PropertyList as CplPropertyList, Property, Msg)

from pymetis.utils.dummy import make_cpl_property


class Hdu:
    """
    A loose association of a header and data
    """
    def __init__(self,
                 header: CplPropertyList,
                 data: Optional[CplImage | CplTable],
                 *,
                 name: Optional[str] = None, # FixMe this should not really be optional for creation as opposed to loading
                 klass: Optional[type[CplImage | CplTable]] = None,
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
        self.klass = klass if klass is not None else type(data) if data is not None else None
        self.extno = extno

        self.name = name

        self.header.del_regexp(r'EXTNAME', True)
        self.header.append(make_cpl_property('EXTNAME', name))

        Msg.debug(self.__class__.__qualname__,
                  f"Created a HDU '{self.name}' with extno={self.extno}, class is {self.klass}")

    def __repr__(self) -> str:
        return f"<HDU {self.name=}, {self.extno=}, {self.klass=}>"

    def save(self, filename):
        Msg.info(self.__class__.__name__,
                 f"Saving HDU '{self.name}' to '{filename}'")

        # FixMe this is ugly as hell, but works
        if self.klass == CplImage:
            self.data.save(filename, self.header, cpl.core.io.EXTEND)
        elif self.klass == CplTable:
            # Here the signature is (primary_header, header, filename, mode) for whatever reason...
            # FixMe What if there are multiple tables? Is primary header overwritten or what?
            self.data.save(self.header, self.header, filename, cpl.core.io.EXTEND)


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