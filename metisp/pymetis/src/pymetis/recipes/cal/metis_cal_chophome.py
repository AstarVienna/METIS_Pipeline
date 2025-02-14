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

import re
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.inputs import RawInput
from pymetis.inputs.common import (LinearityInput, GainMapInput,
                                   PersistenceMapInput, BadpixMapInput,
                                   PinholeTableInput)
from pymetis.base.product import PipelineProduct
from pymetis.prefab.rawimage import RawImageProcessor


class MetisCalChophomeImpl(RawImageProcessor):  # TODO replace parent class?
    """Implementation class for metis_cal_chophome"""
    target = "LM_CHOPHOME"

    class InputSet(RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_CHOPHOME_RAW")

        class BackgroundInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.background = self.BackgroundInput(frameset)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.badpix_map = BadpixMapInput(frameset, required=False)
            self.pinhole_table = PinholeTableInput(frameset, required=True)

            self.inputs |= {self.background, self.linearity, self.gain_map,
                            self.badpix_map, self.persistence, self.pinhole_table}


    class ProductCombined(PipelineProduct):
        """
        Final product: combined, background-subtracted images of the WCU source
        """
        _group = cpl.ui.Frame.FrameGroup.PRODUCT
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return "LM_CHOPHOME_COMBINED"

        @property
        def output_file_name(self) -> str:
            return f"{self.category}.fits"

        @property
        def tag(self) -> str:
            return rf"{self.category}"


    class ProductBackground(PipelineProduct):
        """
        Intermediate product: the instrumental background (WCU OFF)
        """
        _group = cpl.ui.Frame.FrameGroup.PRODUCT
        _level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return "LM_CHOPHOME_BACKGROUND"

        @property
        def output_file_name(self) -> str:
            return f"{self.category}.fits"

        @property
        def tag(self) -> str:
            return rf"{self.category}"


    def process_images(self) -> [PipelineProduct]:
        """do something"""

        background_hdr = cpl.core.PropertyList()
        bg_images = self.load_images(self.inputset.background.frameset)
        background_img = self.combine_images(bg_images, "median")
        # TODO: define usedframes

        combined_hdr = cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)
        raw_images.subtract_image(background_img)
        combined_img = self.combine_images(raw_images, "median")

        return [
            self.ProductCombined(self, combined_hdr, combined_img),
            self.ProductBackground(self, background_hdr, background_img),
        ]

    def load_images(self, frameset: cpl.ui.FrameSet) -> cpl.core.ImageList:
        """Load an imagelist from a FrameSet

        This is a temporary implementation that should be generalized to the
        entire pipeline package. It uses cpl functions - these should be
        replaced with hdrl functions once they become available, in order
        to use uncertainties and masks.
        """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(frameset):
            Msg.info(self.__class__.__qualname__,
                     f"Processing input frame #{idx}: {frame.file!r}...")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output


class MetisCalChophome(MetisRecipe):
    """Determine chopper home position
    """
    # Recipe information
    _name: str = "metis_cal_chophome"
    _version: str = "0.1"
    _author: str = "Oliver Czoske, A*"
    _email: str = "oliver.czoske@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Determination of chopper home position"
    _description: str = """\
    Determine the chopper home position from LM-imaging of the WCU pinhole mask.

        Inputs
            LM_CHOPHOME_RAW: Raw LM band images [1-n]
            LM_WCU_RAW_OFF:  Background images with WCU black-body closed [1-n]
            GAIN_MAP_2RG:    Gain map for 2RG detector
            LINEARITY_2RG:   Linearity map for 2RG detector
            BADPIX_MAP_2RG:  Bad-pixel map for 2RG detector [optional]
            PINHOLE_TABLE:   Table with location of pinhole on mask
            PERSISTENCE_MAP: Persistence map [optional]

        Outputs
            LM_CHOPHOME_BACKGROUND: Average of background images (WCU_OFF)
            LM_CHOPHOME_COMBINED: Stacked background-subtracted images of pinhole mask
                                  The chopper offset is in the header.

        Algorithm
           The position of the pinhole image on the detector is measured from the stacked
           background-subtracted images. The measured position is compared to the WFS
           metrology to give the chopper home position.
    """

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_cal_chophome.stacking.method",
            context="metis_cal_chophome",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])     # no parameters defined in DRLD

    implementation_class = MetisCalChophomeImpl
