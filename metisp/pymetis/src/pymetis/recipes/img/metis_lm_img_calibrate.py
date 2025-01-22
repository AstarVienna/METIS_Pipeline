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
from typing import Dict

from pymetis.inputs.common import RawInput, LinearityInput, BadpixMapInput, PersistenceMapInput, GainMapInput
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput, SinglePipelineInput
from pymetis.prefab.rawimage import RawImageProcessor


class MetisLmImgCalibrateImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"LM_SCI_BKG_SUBTRACTED")

        class PinholeTableInput(SinglePipelineInput):
            _tags = re.compile(r"FLUXCAL_TAB")
            _title = "flux table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        class PinholeTableInput(SinglePipelineInput):
            _tags = re.compile(r"LM_DISTORTION_TAB")
            _title = "distortion table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.flux_table = SinglePipelineInput(frameset,
                                                     tags=re.compile(r"FLUXCAL_TAB"),
                                                     title="pinhole table",
                                                     group=cpl.ui.Frame.FrameGroup.CALIB)
            
            self.distortion_table = SinglePipelineInput(frameset,
                                                     tags=re.compile(r"LM_DISTORTION_TAB"),
                                                     title="distortion table",
                                                     group=cpl.ui.Frame.FrameGroup.CALIB)
          
            
            self.inputs += [self.flux_table, self.distortion_table]

    class ProductLmSciCalibrated(PipelineProduct):
        category = rf"LM_SCI_CALIBRATED"
        tag = category
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE


    def process_images(self) -> Dict[str, PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")

        self.products = {
            product.category: product(self, self.header, combined_image)
            for product in [self.ProductLmSciCalibrated]
        }
        return self.products


class MetisLmImgCalibrate(MetisRecipe):
    _name = "metis_lm_img_calibrate"
    _version = "0.1"
    _author = "Chi-Hung Yan"
    _email = "chyan@asiaa.sinica.edu.tw"
    _synopsis = "Determine optical distortion coefficients for the LM imager."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_calibrate.stacking.method",
            context="metis_lm_img_calibrate",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgCalibrateImpl