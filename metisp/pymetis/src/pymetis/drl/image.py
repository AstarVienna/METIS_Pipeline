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

from typing import Optional

import cpl
from cpl.core import (Image as CplImage,
                      Type as CplType,
                      Mask as CplMask)


def zeros_like(
        image: CplImage,
        bpm: Optional[CplMask] = None,
        *,
        new_type: Optional[CplType] = None,
    ) -> CplImage:
    """
    Create a new CPL Image with the same size as image, optionally with a new underlying type

    # FixMe [Martin] This should ideally be added to CPL:
    # FixMe zeros_like that preserves size but allow you to override the type.
    """
    temp = cpl.core.Image.zeros_like(image)

    if bpm is None:
        temp.bpm = CplMask(image.height, image.width)
    else:
        temp.bpm = bpm

    if new_type is None:
        return temp
    else:
        return temp.cast(new_type)
