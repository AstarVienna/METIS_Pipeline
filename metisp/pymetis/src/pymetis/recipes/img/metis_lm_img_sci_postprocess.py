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

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import PipelineInputSet, SinglePipelineInput


class MetisLmImgSciPostProcessImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class CalibratedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_SCI_CALIBRATED")
            _title = "LM band image after flux calibration"
            _group = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)

            self.calibrated = self.CalibratedInput(frameset)

            self.inputs |= {self.calibrated}
            

    class ProductLmImgSciCoadd(PipelineProduct):
        category = rf"LM_SCI_COADD"
        _tag = category
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> [PipelineProduct]:
        header = cpl.core.PropertyList.load(self.inputset.calibrated.frame.file, 0)
        combined_image = self._create_dummy_image()

        product_coadd = self.ProductLmImgSciCoadd(self, header, combined_image)

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

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])
    implementation_class = MetisLmImgSciPostProcessImpl