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

class MetisCalChophomeImpl(RawImageProcessor):  # TODO parent class
    """Implementation class for metis_cal_chophome"""
    target = "LM_CHOPHOME"

    class InputSet(RawImageProcessor.InputSet):
        """Inputs for metis_cal_chophome"""
        class RawInput(RawInput):
            _tags = re.compile(r"LM_CHOPHOME_RAW")

        class DarkInput(RawInput):
            _tags = re.compile(r"LM_WCU_OFF_RAW")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.badpixmap = BadpixMapInput(frameset, required=False)
            self.pinhole_table = PinholeTableInput(frameset, required=True)

            self.inputs += [self.linearity, self.gain_map, self.badpixmap,
                            self.persistence]


    class ProductCombined(PipelineProduct):
        """
        Final product: combined, background-subtracted images of the WCU source
        """
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

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
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return "LM_CHOPHOME_BACKGROUND"

        @property
        def output_file_name(self) -> str:
            return f"{self.category}.fits"

        @property
        def tag(self) -> str:
            return rf"{self.category}"


    def process_images(self) -> Dict[str, PipelineProduct]:
        """do something"""

        header = cpl.core.PropertyList()
        images = self.load_raw_images()
        image = self.combine_images(images, "add")

        self.products = {
            rf"{self.target}_COMBINED": self.ProductCombined(self, header, image),
            rf"{self.target}_BACKGROUND": self.ProductBackground(self, header, image),
        }
        return self.products


class MetisCalChophome(MetisRecipe):
    """Determine chopper home position
    """
    # Recipe information
    _name = "metis_cal_chophome"
    _version = "0.1"
    _author = "Oliver Czoske, A*"
    _email = "oliver.czoske@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Determination of chopper home position"
    _description = """\
    The recipe determines the chopper home position from LM-imaging of the WCU pinhole mask.
    It measures the position of the pinhole image on the detector and compares to the WFS
    metrology."""

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
