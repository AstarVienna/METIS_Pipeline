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

from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.products import PipelineProduct, TargetSpecificProduct
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.headers.header import Header, HeaderProCatg, HeaderInsOpti3Name, HeaderInsOpti9Name, HeaderInsOpti10Name, HeaderDrsFilter


class MetisLmImgBackgroundImpl(MetisRecipeImpl):
    detector = '2RG'

    class InputSet(PipelineInputSet):
        class BasicReducedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_(?P<target>SCI|STD)_BASIC_REDUCED")
            _title = "Detrended exposure"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _description = "Detrended exposure of the LM image mode."

            # @classmethod
            # def description(cls) -> str:
            #     target = {
            #         'SCI': 'Science grade',
            #         'STD': 'Standard',
            #     }.get(cls._target, '{target}')
            #     return f"{target} detrended exposure of the LM image mode."

        class SkyBasicReducedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_SKY_BASIC_REDUCED")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Sky basic-reduced exposure"
            _description: str = "Detrended exposure of the sky."

    class ProductBkg(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords: {Header} = {HeaderProCatg, HeaderInsOpti3Name, HeaderInsOpti9Name, HeaderInsOpti10Name, HeaderDrsFilter}

        @classmethod
        def description(cls):
            target = {
                'SCI': 'science',
                'STD': 'standard',
            }.get(cls.target(), '{target}')
            return f"Thermal background of {target} LM exposures."

        @classmethod
        def tag(cls):
            return f"LM_{cls.target():s}_BKG"

    class ProductBkgSubtracted(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords: {Header} = {HeaderProCatg, HeaderInsOpti3Name, HeaderInsOpti9Name, HeaderInsOpti10Name, HeaderDrsFilter}

        @classmethod
        def description(cls):
            target = {
                'SCI': 'science',
                'STD': 'standard',
            }.get(cls.target(), '{target}')
            return f"Thermal background subtracted images of {target} LM exposures."

        @classmethod
        def tag(cls):
            return f"LM_{cls.target():s}_BKG_SUBTRACTED"

    class ProductObjectCat(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE
        _oca_keywords: {Header} = {HeaderProCatg, HeaderDrsFilter}

        @classmethod
        def description(cls):
            target = {
                'SCI': 'science',
                'STD': 'standard',
            }.get(cls.target(), '{target}')
            return f"Catalog of masked objects in {target} LM exposures."

        @classmethod
        def tag(cls):
            return rf"LM_{cls.target():s}_OBJECT_CAT"

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        target = self.inputset.tag_parameters['target']
        image = self._create_dummy_image()

        product_bkg = self.ProductBkg(self, self.header, image)
        product_bkg_subtracted = self.ProductBkgSubtracted(self, self.header, image)
        product_object_cat = self.ProductObjectCat(self, self.header, image)

        return [product_bkg, product_bkg_subtracted, product_object_cat]

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        return {
            'STD': MetisLmImgBackgroundStdImpl,
            'SCI': MetisLmImgBackgroundSciImpl,
        }[self.inputset.target]


class MetisLmImgBackgroundStdImpl(MetisLmImgBackgroundImpl):
    class InputSet(TargetStdMixin, MetisLmImgBackgroundImpl.InputSet):
        class BasicReducedInput(TargetStdMixin, MetisLmImgBackgroundImpl.InputSet.BasicReducedInput): pass

    class ProductBkg(TargetStdMixin, MetisLmImgBackgroundImpl.ProductBkg): pass
    class ProductObjectCat(TargetStdMixin, MetisLmImgBackgroundImpl.ProductObjectCat): pass
    class ProductBkgSubtracted(TargetStdMixin, MetisLmImgBackgroundImpl.ProductBkgSubtracted): pass


class MetisLmImgBackgroundSciImpl(MetisLmImgBackgroundImpl):
    class InputSet(TargetSciMixin, MetisLmImgBackgroundImpl.InputSet):
        class BasicReducedInput(TargetSciMixin, MetisLmImgBackgroundImpl.InputSet.BasicReducedInput): pass

    class ProductBkg(TargetSciMixin, MetisLmImgBackgroundImpl.ProductBkg): pass
    class ProductObjectCat(TargetSciMixin, MetisLmImgBackgroundImpl.ProductObjectCat): pass
    class ProductBkgSubtracted(TargetSciMixin, MetisLmImgBackgroundImpl.ProductBkgSubtracted): pass


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

    _matched_keywords: {Header} = {HeaderDrsFilter}
    _algorithm = """Average all or SKY exposures with object rejection
    Subtract background"""

    implementation_class = MetisLmImgBackgroundImpl
