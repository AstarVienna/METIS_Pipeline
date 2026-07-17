"""
This file is part of the A* Pipeline.
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

import functools
import operator

import numpy as np

from cpl.core import (Image as CplImage,
                      Mask as CplMask,
                      Type as CplType,)

class Mask:
    def __init__(self, data: CplImage):
        self.data: CplImage = data

    @classmethod
    def from_cpl_mask(cls, mask: CplMask, bit: int = 1):
        return Mask.from_cpl_masks({bit: mask})

    @classmethod
    def from_cpl_masks(cls, masks: dict[int, CplMask]):
        """
        Create from a dictionary of masks in the form {bit: mask}
        """
        # First ensure that all masks are the same size
        sizes = list(set([(mask.width, mask.height) for mask in masks.values()]))
        if len(sizes) != 1:
            raise ValueError("All masks must have the same width and height")

        # Sum them all with appropriate bits
        empty = CplImage(np.ndarray(shape=sizes[0]), dtype=CplType.INT)
        data = functools.reduce(operator.or_, [bit * mask.data for bit, mask in masks.items()], empty)
        return Mask(data=data)

    def flatten(self):
        """
        Flatten a 32-bit mask into a 1-bit mask for use with CPL / HDRL functions (see DRLD section 3.5.3).

        A pixel is bad <=> any of the bits in the mask is nonzero,
        hence only `0x00000000` represents a valid pixel.
        """
        binary = (self.data.as_array() != 0).astype(np.bool_)
        return CplMask(binary)
