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
import enum
from typing import Literal

import cpl
import numpy as np
from numpy._typing import NDArray

from pymetis.engine.core.classes.mask import Mask


class InstrumentDescription:
    class MaskFlags(enum.IntFlag):
        """
        This class provides meanings for the mask bits.

        Flags are stored in a `Mask`, whose backing store is a signed 32-bit
        integer, so bit 31 is the sign bit and must stay unused. Every flag
        must therefore fit within bits 0-`Mask.MAX_FLAG_BIT`; this is enforced
        for all subclasses at definition time.
        """

        def __init_subclass__(cls, **kwargs):
            super().__init_subclass__(**kwargs)
            for member in cls:
                if member.value.bit_length() > Mask.MAX_FLAG_BIT + 1:
                    raise ValueError(
                        f"{cls.__qualname__}.{member.name} = 0x{member.value:08x} "
                        f"uses a bit above {Mask.MAX_FLAG_BIT}; bit 31 is reserved "
                        f"as the sign bit of the signed int32 mask storage."
                    )


class Metis(InstrumentDescription):
    border_2rg = 64
    border_geo = 28
    border_ifu_x = 64
    border_ifu_y = 32

    class MaskFlags(InstrumentDescription.MaskFlags):
        # det_dark
        BAD = 0x0001                        # bad pixel
        COLD = 0x0002                       # cold pixel
        HOT = 0x0004                        # hot pixel

        # det_lingain
        TOO_FEW_SAMPLES = 0x0010
        UNDERDETERMINED = 0x0020            # underdetermined for fit
        CONVERGENCE_FAILURE = 0x0040        # failed to converge in linearity determination
        LINEARITY_OUTLIER = 0x0080          # outlier

        # persistence
        PERSISTENCE_AFFECTED = 0x0100

        # custom masks
        EDGE = 0x00010000                   # too close to the detector edge
        NOT_AN_ORDER = 0x00010010           # pixel masked because it does not belong to a spectral order

    def get_detector_size(self, tech: str) -> tuple[int, int]:
        if 'LM' in tech:
            return (2048, 2048)
        elif 'N' in tech:
            return (2048, 2048)
        elif 'IFU' in tech:
            return (2048, 2048)
        else:
            raise cpl.core.IllegalInputError(f"Unknown ESO DPR TECH {tech}")

    def get_detector_mask(self, tech: str, detector: Literal[1, 2, 3, 4]) -> NDArray[bool]:
        """
        A mask to ignore the masked pixels at the edge of the detector.
        EXTERNAL CALIBRATION, in case of the IFU the mask needs to only cover the visible traces.
        This needs to depend on detector because the LMS mask varies.
        """
        det_width, det_height = self.get_detector_size(tech)
        xx, yy = np.meshgrid(np.arange(det_width), np.arange(det_height))

        if 'LM' in tech:
            return (((xx >= self.border_2rg) & (xx < (det_width - self.border_2rg))) &
                    ((yy >= self.border_2rg) & (yy < (det_height - self.border_2rg))))
        elif 'N' in tech:
            return (((xx >= self.border_geo) & (xx < (det_width - self.border_geo))) &
                    ((yy >= self.border_geo) & (yy < (det_height - self.border_geo))))
        elif 'IFU' in tech:
            # Detector 1 and 2 are butted against each other in 1 dimension. Same for detectors 3 and 4.
            if detector in [1, 3]:
                return (((xx >= self.border_ifu_x) & (xx < det_width)) &
                        ((yy >= self.border_ifu_y) & (yy < (det_height - self.border_ifu_y))))
            elif detector in [2, 4]:
                return (((xx >= 0) & (xx < (det_width - self.border_ifu_x))) &
                        ((yy >= self.border_ifu_y) & (yy < (det_width - self.border_ifu_y))))
            else:
                raise cpl.core.IllegalInputError(f"Detector ID {detector} not recognised")
        else:
            raise cpl.core.IllegalInputError(f"Unknown ESO DPR TECH {tech}")
