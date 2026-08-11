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

from typing import Self

import numpy as np

import cpl
from cpl.core import (Image as CplImage,
                      Mask as CplMask,
                      Type as CplType,)


class DataQuality:
    """
    A 31-bit data-quality layer: every pixel holds a bitfield whose individual
    bits flag independent defect classes. The meaning of each bit is declared
    by the instrument (see ``InstrumentDescription.MaskFlags``).
    This class only provides the mechanism but no domain logic.

    CPL and HDRL only understand 1-bit masks -- ``cpl.core.Mask`` is a plain
    boolean array -- so the individual bits cannot be carried through a
    CPL/HDRL call. The rich bitfield is kept here as the source of truth and
    collapsed to a binary bad-pixel mask only at that boundary via
    :meth:`flatten` or :meth:`__getitem__` (also see DRLD section 3.5.3).

    Bit arithmetic is done in numpy because ``cpl.core.Image`` exposes no
    bitwise operators; the canonical store is a ``CplType.INT`` image so it
    round-trips through the ``.DQ`` FITS extension unchanged. Note that
    ``CplType.INT`` is a *signed* int32, so bit 31 is the sign bit and may not be used.
    Only     bits 0-30 should be used for flags.
    """


    def __init__(self, data: "CplImage | CplMask | DataQuality"):
        """
        Build a 32-bit mask from, in the manner of a C++ converting constructor:

        - a ``CplImage`` holding the raw bitfield,
        - a 1-bit ``CplMask`` (promoted to bit 0, i.e. a generic bad pixel), or
        - another ``Mask`` (copy constructor).

        The pixel data is always copied and coerced to ``int32``, so the new
        mask shares no buffer with its source.
        """
        if isinstance(data, DataQuality):
            source = data._array()
        elif isinstance(data, CplMask):
            source = np.asarray(data).astype(np.int32)
        elif isinstance(data, CplImage):
            source = data.as_array().astype(np.int32)
        else:
            raise TypeError(f"Cannot build a Mask from {type(data)}")
        self.data: CplImage = CplImage(source, dtype=CplType.INT)

    def _array(self) -> np.ndarray:
        """The bitfield as an ``int32`` numpy array (shape ``(height, width)``)."""
        return self.data.as_array().astype(np.int32)

    @property
    def width(self) -> int:
        return self.data.width

    @property
    def height(self) -> int:
        return self.data.height

    def copy(self) -> "DataQuality":
        """An independent copy that shares no pixel buffer with this mask."""
        return DataQuality(self)

    @classmethod
    def from_cpl_mask(cls, mask: CplMask, bit: int):
        """
        Create a new Mask from a CPL mask.
        """
        if bit.bit_count() != 1:
            raise ValueError("Only one bit must be set")

        return cls.from_cpl_masks({bit: mask})

    @classmethod
    def from_cpl_masks(cls, masks: dict[int, CplMask]):
        """
        Create from a dictionary of 1-bit CPL masks in the form {bit: mask}.
        """
        # CPL images have no bitwise operators, so combine in numpy: each mask
        # is a boolean array and its bit is OR-ed in wherever the mask is set.
        arrays = {bit: np.asarray(mask).astype(bool) for bit, mask in masks.items()}

        # First ensure that all masks are the same size.
        shapes = {array.shape for array in arrays.values()}
        if len(shapes) != 1:
            raise cpl.hdrl.core.IncompatibleInputError(f"All masks must have the same width and height, got {shapes}")

        combined = np.zeros(shapes.pop(), dtype=np.int32)
        for bit, selected in arrays.items():
            combined[selected] |= bit
        return cls(CplImage(combined, dtype=CplType.INT))

    @classmethod
    def zeros_like(cls, source) -> Self:
        return cls(CplImage.zeros_like(source))

    def flatten(self) -> CplMask:
        """
        Flatten a 32-bit mask into a 1-bit mask for use with CPL / HDRL functions (see DRLD section 3.5.3).

        A pixel is bad <=> any of the bits in the mask is nonzero,
        hence only `0x00000000` represents a valid pixel.
        """
        return CplMask(self._array() != 0)

    def __and__(self, other: Self) -> Self:
        """Intersection of two masks: bitwise-AND of their flags."""
        return DataQuality(CplImage(self._array() & other._array(), dtype=CplType.INT))

    def __or__(self, other: Self) -> Self:
        """Union of two masks: bitwise-OR of their flags."""
        return DataQuality(CplImage(self._array() | other._array(), dtype=CplType.INT))

    def add(self, mask: CplMask, bit: int) -> Self:
        """Add a CPL mask to this mask as bit `bit`."""
        combined = self._array()
        combined[np.asarray(mask).astype(bool)] |= bit
        self.data = CplImage(combined, dtype=CplType.INT)
        return self

    def __getitem__(self, bits: 'InstrumentDescription.MaskFlags') -> CplMask:
        """
        Flatten the mask to 1-bit, using only selected bits, and return a corresponding CplMask.

        Usage
        -----
        cpl_mask: CplMask = mask[Instrument.MaskFlags.COLD | Instrument.MaskFlags.HOT | Instrument.MaskFlags.BAD]

        Extracts all pixels that are (hot, cold or bad) and flattens that to a 1-bit CPL mask.
        """
        # `& bits` isolates the selected bits
        # `!= 0` collapses it to a boolean CplMask
        return CplMask((self._array() & bits) != 0)
