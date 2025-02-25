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

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct, TargetSpecificProduct
from pymetis.inputs import PipelineInputSet, SinglePipelineInput


class MetisLmImgBackgroundImpl(MetisRecipeImpl):
    detector = '2RG'

    class InputSet(PipelineInputSet):
        class BasicReducedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_(?P<target>SCI|STD)_BASIC_REDUCED")
            _title = "Detrended exposure"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _description = "Science grade detrended exposure of the LM image mode."
                           # "Standard detrended exposure of the LM image mode.")

        class SkyBasicReducedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_(?P<target>SKY)_BASIC_REDUCED")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Sky basic-reduced exposure"
            _description = "Detrended exposure of the sky."

    class ProductBkg(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

        @classmethod
        def description(cls):
            target = 'science' if cls.target == 'SCI' else 'standard'
            return f"Thermal background of {target} LM exposures."

        @classmethod
        def tag(cls):
            return f"LM_{cls.target():s}_BKG"

    class ProductBkgSubtracted(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

        @classmethod
        def description(cls):
            target = 'science' if cls.target == 'SCI' else 'standard'
            return f"Thermal background subtracted images of {target} LM exposures."

        @classmethod
        def tag(cls):
            return f"LM_{cls.target():s}_BKG_SUBTRACTED"

    class ProductObjectCat(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE
        oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def description(cls):
            target = 'science' if cls.target == 'SCI' else 'standard'
            return f"Catalog of masked objects in {target} LM exposures."

        @classmethod
        def tag(cls):
            return rf"LM_{cls.target():s}_OBJECT_CAT"

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        target = self.inputset.tag_parameters['target']
        image = self._create_dummy_image()

        product_bkg = self.ProductBkg(self, self.header, image, target=target)
        product_bkg_subtracted = self.ProductBkgSubtracted(self, self.header, image, target=target)
        product_object_cat = self.ProductObjectCat(self, self.header, image, target=target)

        return [product_bkg, product_bkg_subtracted, product_object_cat]


class MetisLmImgBackground(MetisRecipe):
    _name: str = "metis_lm_img_background"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Basic reduction of raw exposures from the LM-band imager"
    _description: str = ""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_background.stacking.method",
            context="metis_lm_img_background",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    _matched_keywords: {str} = {'DRS.FILTER'}
    _algorithm = """Average all or SKY exposures with object rejection
    Subtract background"""

    implementation_class = MetisLmImgBackgroundImpl
