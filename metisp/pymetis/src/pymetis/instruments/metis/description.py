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
from typing import Literal

import cpl
import numpy as np
from numpy._typing import NDArray


class InstrumentDescription:
    class MaskFlags:
        pass


class Metis(InstrumentDescription):
    border_2rg = 64
    border_geo = 28
    border_ifu_x = 64
    border_ifu_y = 32

    class MaskFlags:
        # det_dark
        BAD = 0x0001
        COLD = 0x0002
        HOT = 0x0004

        # det_lingain
        TOO_FEW_SAMPLES = 0x0010
        UNDERDETERMINED = 0x0020
        CONVERGENCE_FAILURE = 0x0040
        LINEARITY_OUTLIER = 0x0080

        # persistence
        PERSISTENCE_AFFECTED = 0x0100

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
