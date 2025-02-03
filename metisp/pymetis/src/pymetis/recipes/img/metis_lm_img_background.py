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
from pymetis.base.product import PipelineProduct, TargetSpecificProduct
from pymetis.inputs import RawInput, PipelineInputSet


class MetisLmImgBackgroundImpl(MetisRecipeImpl):
    detector = '2RG'

    class InputSet(PipelineInputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"LM_(?P<target>SCI|STD)_BASIC_REDUCED")

        class SkyInput(RawInput):
            _tags = re.compile(r"LM_(?P<target>SKY)_BASIC_REDUCED")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.basic_reduced = self.RawInput(frameset)
            self.sky_reduced = self.SkyInput(frameset)
            
            # We need to register the inputs (just to be able to do `for x in self.inputs:`)
            self.inputs |= {self.basic_reduced, self.sky_reduced}

    class ProductBkg(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self):
            return f"LM_{self.target:s}_BKG"

    class ProductBkgSubtracted(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self):
            return f"LM_{self.target:s}_BKG_SUBTRACTED"

    class ProductObjectCat(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE

        @property
        def tag(self):
            return rf"LM_{self.target:s}_OBJECT_CAT"

    def process_images(self) -> Dict[str, PipelineProduct]:
        raw_images = cpl.core.ImageList()

        target = self.inputset.basic_reduced.get_target_name(self.inputset.basic_reduced.frameset)
        combined_image = self._create_dummy_image()
        
        self.products = {
            product.category: product(self, self.header, combined_image, target=target)
            for product in [self.ProductBkg, self.ProductBkgSubtracted, self.ProductObjectCat]
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
            name="metis_lm_img_background.stacking.method",
            context="metis_lm_img_background",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    implementation_class = MetisLmImgBackgroundImpl
