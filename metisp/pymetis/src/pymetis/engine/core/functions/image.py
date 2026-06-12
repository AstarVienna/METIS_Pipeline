"""
This file is part of an A* Pipeline.
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


from typing import Optional, ClassVar

import cpl
from cpl.core import (Image as CplImage,
                      Type as CplType,
                      PropertyList as CplPropertyList)

from pymetis.engine.dataitems import Hdu


def zeros_like(image: CplImage, new_type: Optional[CplType] = None):
    """
    Create a new CPL Image with the same size as image, optionally with a new underlying type

    # FixMe [Martin] This should ideally be added to CPL:
    # FixMe zeros_like that preserves size but allow you to override the type.
    """
    temp = cpl.core.Image.zeros_like(image)
    if new_type is None:
        return temp
    else:
        return temp.cast(new_type)


class EnhancedImage:
    """
    A high-level image object that encapsulates a data layer, error layer and data dq layer.
    Should be independent of CPL and FITS quirks but support loading and saving.
    """

    sci_prefix: ClassVar[str] = 'SCI'
    err_prefix: ClassVar[str] = 'ERR'
    dq_prefix: ClassVar[str] = 'DQ'

    def __init__(self,
                 image: CplImage,
                 error: Optional[CplImage] = None,
                 dq: Optional[CplImage] = None,
                 *,
                 prefix: str,
                 header_image: Optional[CplPropertyList] = None,
                 header_error: Optional[CplPropertyList] = None,
                 header_dq: Optional[CplPropertyList] = None):
        self.prefix = prefix
        self.image = Hdu(header_image, image, name=rf'{self.prefix}.{self.sci_prefix}')
        self.error = Hdu(header_error, error, name=rf'{self.prefix}.{self.err_prefix}')
        self.dq = Hdu(header_dq, dq, name=rf'{self.prefix}.{self.dq_prefix}')

    def as_list(self) -> list[Hdu]:
        return [
            self.image,
            self.error,
            self.dq,
        ]