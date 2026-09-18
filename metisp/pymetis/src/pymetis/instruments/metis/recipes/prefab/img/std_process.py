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
from pymetis.engine.core.functions.dummy import create_dummy_table, create_dummy_header

from pymetis.instruments.metis.inputs import FluxstdCatalogInput
from pymetis.instruments.metis.inputs import RawInput
from pymetis.instruments.metis.mixins import TargetStdMixin
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor
from pymetis.instruments.metis import qc
from pymetis.instruments.metis import dataitems


class MetisImgStdProcessImpl(TargetStdMixin, RawImageProcessor, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class RawInput(RawInput):
            Item = dataitems.BackgroundSubtracted

        raw: RawInput
        fluxstd_catalog: FluxstdCatalogInput

    class ProductSet(PipelineProductSet):
        ImgFluxCalTable = dataitems.FluxCalTable
        ImgStdCombined = dataitems.Combined

    class Qc(QcParameterSet):

        Fwhm = qc.std_process.QcStdFwhm
        Airmass = qc.std_process.QcStdAirmass
        BackgroundRms = qc.std_process.QcImgStdBackgroundRms
        PeakCounts = qc.std_process.QcStdPeakCounts
        ApertureCounts = qc.std_process.QcStdApertureCounts
        Strehl = qc.std_process.QcStdStrehl
        Ellipticity = qc.std_process.QcStdEllipticity
        FluxConversion = qc.std_process.QcStdFluxConversion
        Sensitivity = qc.std_process.QcSensitivity
        AreaSensitivity = qc.std_process.QcAreaSensitivity
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

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.Airmass(None),
            self.Qc.ApertureCounts(None),
            self.Qc.AreaSensitivity(None),
            self.Qc.BackgroundRms(None),
            self.Qc.Ellipticity(None),
            self.Qc.FluxConversion(None),
            self.Qc.Fwhm(None),
            self.Qc.PeakCounts(None),
            self.Qc.Sensitivity(None),
            self.Qc.Strehl(None),
        ))

        return {product_fluxcal, product_combined}