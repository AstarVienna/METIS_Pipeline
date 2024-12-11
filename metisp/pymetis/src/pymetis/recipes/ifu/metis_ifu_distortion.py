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

import cpl
from cpl.core import Msg
from typing import Dict, Literal

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput, MasterDarkInput, BadpixMapInput, SinglePipelineInput, GainMapInput
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuDistortionImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags = re.compile(r"IFU_DISTORTION_RAW")

        class MasterDarkInput(MasterDarkInput):
            _tags = re.compile(r"MASTER_DARK_IFU")

        class PinholeTableInput(SinglePipelineInput):
            _tags = re.compile(r"PINHOLE_TABLE")
            _title = "pinhole table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.badpix_map = BadpixMapInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.pinhole_table = self.PinholeTableInput(frameset)

    class ProductIfuDistortionTable(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    class ProductIfuDistortionReduced(PipelineProduct):
        category = rf"IFU_DIST_REDUCED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        masterdark_image = cpl.core.Image.load(self.inputset.master_dark.file)
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_image.subtract(masterdark_image)
            raw_images.append(raw_image)

        method = self.parameters["metis_ifu_calibrate.stacking.method"].value
        combined_image = self.combine_images(raw_images, method)

        self.products = {
            product.category: product(self, self.header, combined_image, detector_name=self.detector_name)
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products


class MetisIfuDistortion(MetisRecipe):
    _name = "metis_ifu_distortion"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([])
    implementation_class = MetisIfuDistortionImpl
