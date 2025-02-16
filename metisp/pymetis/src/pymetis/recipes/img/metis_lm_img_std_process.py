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

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.inputs.common import FluxTableInput


class MetisLmImgsStdProcessImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SubtractedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_STD_BKG_SUBTRACTED")
            _title = "thermal-background-subtracted LM band image"
            _group = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.subtracted = self.SubtractedInput(frameset)
            self.fluxstd_table = FluxTableInput(frameset)

            self.inputs |= {self.subtracted, self.fluxstd_table}

    #import pdb ; pdb.set_trace()
    class ProductLmImgFluxCalTable(PipelineProduct):
        _tag = r"FLUXCAL_TAB"
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.TABLE

    class ProductLmImgStdCombined(PipelineProduct):
        _tag = r"LM_STD_COMBINED"
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> [PipelineProduct]:
        header = cpl.core.PropertyList.load(self.inputset.subtracted.frame.file, 0)
        combined_image = self._create_dummy_image()

        product_fluxcal = self.ProductLmImgFluxCalTable(self, header, combined_image)
        product_combined = self.ProductLmImgStdCombined(self, header, combined_image)

        return [product_fluxcal, product_combined]


class MetisLmImgStdProcess(MetisRecipe):
    _name: str = "metis_lm_img_std_process"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine the conversion factor between detector counts and physical source flux"
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

    implementation_class = MetisLmImgsStdProcessImpl
