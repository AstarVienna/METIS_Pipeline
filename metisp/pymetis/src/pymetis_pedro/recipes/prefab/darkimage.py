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

from abc import ABC

import cpl
from cpl.core import Msg, Image

from pymetis.classes.dataitems import DataItem
from pymetis.classes.inputs import PipelineInput
from pymetis.classes.inputs.common import MasterDarkInput
from pymetis.recipes.prefab.rawimage import RawImageProcessor


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    `DarkImageProcessor` is a subclass of `RawImageProcessor` that:
        1. takes a set of raw images to combine
        2. requires a single `master_dark` frame, that will be subtracted from every raw image
        3. combines the raws after subtraction into a single product

    It also provides methods for loading and verification of the dark frame,
    warns if multiple master darks are provided, etc.
    """
    class InputSet(RawImageProcessor.InputSet, abstract=True):
        """
        A DarkImageProcessor's Input is just a raw image processor input with a master dark frame.
        The exact class is not specified at this point -- it must be set by the subclass.
        """
        MasterDarkInput: type[PipelineInput] = MasterDarkInput

    def subtract_dark(self,
                      images: cpl.core.ImageList) -> cpl.core.ImageList:
        """
        Load the associated master dark frame and subtract it from every image in `images`.
        Also automatically marks the master dark as used.

        :param images:
            ImageList to be corrected.
        :return:
            ImageList
        """
        # FixMe: This currently works somehow, but only for one detector.
        # The function should take all three (SCI, ERR, DQ) and for all detectors
        # Or maybe have two functions:
        # - _subtract_single_dark for a single detector
        # - _subtract_darks for all detectors that calls the single one for each of them
        master_dark: Image = self.inputset.master_dark.load_data('DET1.SCI')

        Msg.info(self.__class__.__qualname__,
                 f"Subtracting the master dark from raw images")
        images.subtract_image(master_dark)
        return images