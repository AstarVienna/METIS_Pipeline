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

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import PipelineInputSet, SinglePipelineInput


class MetisLmImgBackgroundImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class LmBasicReducedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_(?P<target>SCI|SKY|STD)_BASIC_REDUCED")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.basic_reduced = self.LmBasicReducedInput(frameset)

            # We need to register the inputs (just to be able to do `for x in self.inputs:`)
            self.inputs |= {self.basic_reduced}

    class ProductBkg(PipelineProduct):
        tag: str = "LM_{target}_BKG"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    class ProductBkgSubtracted(PipelineProduct):
        tag: str = "LM_{target}_BKG_SUBTRACTED"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    class ProductObjectCat(PipelineProduct):
        tag: str = "LM_{target}_OBJECT_CAT"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE

    def process_images(self) -> Dict[str, PipelineProduct]:
        Msg.info(self.__class__.__qualname__, f"Starting processing image attribute.")

        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        image_bkg = cpl.core.Image() # ToDo implementation missing
        image_bkg_subtracted = cpl.core.Image() # ToDo implementation missing
        table_object_cat = cpl.core.Table()

        self.products = {
            self.ProductBkg.tag:
                self.ProductBkg(self, header, image_bkg),
            self.ProductBkgSubtracted.tag:
                self.ProductBkgSubtracted(self, header, image_bkg_subtracted),
            self.ProductObjectCat.tag:
                self.ProductObjectCat(self, header, table_object_cat),
        }

        return self.products


class MetisLmImgBackground(MetisRecipe):
    _name = "metis_lm_img_background"
    _version = "0.1"
    _author = "Chi-Hung Yan"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic reduction of raw exposures from the LM-band imager"
    _description = ""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="background.stacking.method",
            context="background",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    implementation_class = MetisLmImgBackgroundImpl
