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

import copy

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import PipelineInputSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.dummy import create_dummy_table, create_dummy_header

from pymetis.instruments.metis.inputs import FluxstdCatalogInput
from pymetis.instruments.metis.inputs import RawInput
from pymetis.instruments.metis.mixins import TargetStdMixin
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor
from pymetis.instruments.metis.dataitems.background.subtracted import BackgroundSubtracted
from pymetis.instruments.metis.dataitems.combined import Combined
from pymetis.instruments.metis.dataitems.common import FluxCalTable
from pymetis.instruments.metis.qc.std_process import (QcImgStdBackgroundRms, QcStdPeakCounts, QcStdApertureCounts,
                                                      QcStdStrehl, QcStdEllipticity, QcStdFluxConversion,
                                                      QcSensitivity, QcAreaSensitivity)


class MetisImgStdProcessImpl(TargetStdMixin, RawImageProcessor, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class RawInput(RawInput):
            Item = BackgroundSubtracted

        class FluxstdCatalogInput(FluxstdCatalogInput):
            pass

    class ProductSet(PipelineProductSet):
        ImgFluxCalTable = FluxCalTable
        ImgStdCombined = Combined

    class Qc(QcParameterSet):
        BackgroundRms = QcImgStdBackgroundRms
        PeakCounts = QcStdPeakCounts
        ApertureCounts = QcStdApertureCounts
        Strehl = QcStdStrehl
        Ellipticity = QcStdEllipticity
        FluxConversion = QcStdFluxConversion
        Sensitivity = QcSensitivity
        AreaSensitivity = QcAreaSensitivity

    def process(self) -> set[DataItem]:
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        combined_image = self.combine_images(raw_images, "average")
        primary_header = self.inputset.raw.items[0].primary_header

        header_table = create_dummy_header()
        header_combined = create_dummy_header()
        table = create_dummy_table()

        product_combined = self.ProductSet.ImgStdCombined(
            copy.deepcopy(primary_header),
            Hdu(header_combined, combined_image, name='IMAGE'),
        )
        product_fluxcal = self.ProductSet.ImgFluxCalTable(
            copy.deepcopy(primary_header),
            Hdu(header_table, table, name='TABLE')
        )

        return {product_fluxcal, product_combined}