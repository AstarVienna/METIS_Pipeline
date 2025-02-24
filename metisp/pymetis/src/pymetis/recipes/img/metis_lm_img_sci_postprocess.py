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

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput
from pymetis.prefab.rawimage import RawImageProcessor


class MetisLmImgSciPostProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_SCI_CALIBRATED")
            _description = "LM band image with flux calibration, WC coordinate system and distorion information"


    class ProductLmImgSciCoadd(PipelineProduct):
        _tag = rf"LM_SCI_COADD"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        description = "Coadded, mosaiced LM image."

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=0)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")

        product_coadd = self.ProductLmImgSciCoadd(self, self.header, combined_image)

        return [product_coadd]


class MetisLmImgSciPostProcess(MetisRecipe):
    _name: str = "metis_lm_img_sci_postprocess"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Coadd reduced images"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = ['DRS.FILTER']

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_sci_postprocess.stacking.method",
            context="metis_lm_img_sci_postprocess",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgSciPostProcessImpl
