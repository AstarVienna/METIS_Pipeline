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

import cpl

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.background import Background, BackgroundSubtracted
from pymetis.dataitems.img.basicreduced import BasicReduced, LmSkyBasicReduced
from pymetis.dataitems.objectcatalog import ObjectCatalog
from pymetis.classes.mixins import BandLmMixin, Detector2rgMixin
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput


class MetisLmImgBackgroundImpl(MetisRecipeImpl):
    class InputSet(BandLmMixin, Detector2rgMixin, PipelineInputSet):
        class BasicReducedInput(SinglePipelineInput):
            Item = BasicReduced

        class SkyBasicReducedInput(SinglePipelineInput):
            Item = LmSkyBasicReduced

    ProductBkg = Background
    ProductBkgSubtracted = BackgroundSubtracted
    ProductObjectCatalog = ObjectCatalog

    def process(self) -> set[DataItem]:
        raw_images = cpl.core.ImageList()
        image = self._create_dummy_image()
        table = self._create_dummy_table()

        product_bkg = self.ProductBkg(self.header, image)
        product_bkg_subtracted = self.ProductBkgSubtracted(self.header, image)
        product_object_cat = self.ProductObjectCatalog(self.header, table)

        return {product_bkg, product_bkg_subtracted, product_object_cat}


class MetisLmImgBackground(MetisRecipe):
    _name = "metis_lm_img_background"
    _version = "0.1"
    _author = "Chi-Hung Yan, A*"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic reduction of raw exposures from the LM-band imager"
    _description = ""

    parameters = ParameterList([
        ParameterEnum(
            name="metis_lm_img_background.stacking.method",
            context="metis_lm_img_background",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])

    _matched_keywords = {'DRS.FILTER'}
    _algorithm = """Average all or SKY exposures with object rejection
    Subtract background"""

    Impl = MetisLmImgBackgroundImpl
