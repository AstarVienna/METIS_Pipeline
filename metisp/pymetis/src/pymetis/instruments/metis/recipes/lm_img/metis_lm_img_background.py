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

from pymetis.engine.core.parameter import ParameterList, ParameterEnum
from pymetis.engine.core.dummy import create_dummy_table, create_dummy_header
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe
from pymetis.engine.inputs import PipelineInputSet, SinglePipelineInput

from pymetis.instruments.metis.dataitems.background import Background, BackgroundSubtracted
from pymetis.instruments.metis.dataitems.img.basicreduced import BasicReduced, LmSkyBasicReduced
from pymetis.instruments.metis.dataitems.objectcatalog import ObjectCatalog
from pymetis.instruments.metis.mixins import BandLmMixin, Detector2rgMixin
from pymetis.instruments.metis.qc.background import QcLmImgBkgMedian, QcLmImgBkgMedianDeviation
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl


class MetisLmImgBackgroundImpl(BandLmMixin, Detector2rgMixin, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class BasicReducedInput(SinglePipelineInput):
            Item = BasicReduced

        class SkyBasicReducedInput(SinglePipelineInput):
            Item = LmSkyBasicReduced

    class ProductSet(PipelineProductSet):
        Bkg = Background
        BkgSubtracted = BackgroundSubtracted
        ObjectCatalog = ObjectCatalog

    class Qc(QcParameterSet):
        Median = QcLmImgBkgMedian
        MedianDev = QcLmImgBkgMedianDeviation

    def process(self) -> set[DataItem]:
        image = self.inputset.basic_reduced.load_data('DET1.DATA')
        primary_header = self.inputset.basic_reduced.item.primary_header
        table = create_dummy_table()
        header_bkg = create_dummy_header()
        header_bkg_subtracted = create_dummy_header()
        header_object_cat = create_dummy_header()

        product_bkg = self.ProductSet.Bkg(
            create_dummy_header(),
            Hdu(header_bkg, image, name='DET1.DATA'),
        )
        product_bkg_subtracted = self.ProductSet.BkgSubtracted(
            primary_header,
            Hdu(header_bkg_subtracted, image, name='DET1.DATA'),
        )
        product_object_cat = self.ProductSet.ObjectCatalog(
            create_dummy_header(),
            Hdu(header_object_cat, table, name='TABLE'),
        )

        return {product_bkg, product_bkg_subtracted, product_object_cat}


class MetisLmImgBackground(Recipe):
    _name = "metis_lm_img_background"
    _version = "0.1"
    _author = "Chi-Hung Yan, A*"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic reduction of raw exposures from the {band} imager"
    _description = "Something"

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
