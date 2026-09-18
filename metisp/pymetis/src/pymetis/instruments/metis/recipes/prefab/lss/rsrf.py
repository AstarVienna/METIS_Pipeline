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
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header

from pymetis.instruments.metis.dataitems.lss.rsrf import LssRsrfRaw, MedianLssRsrf, MeanLssRsrf, MasterLssRsrf
from pymetis.instruments.metis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.instruments.metis.inputs import (RawInput, OptionalInputMixin, PersistenceMapInput,
                                              GainMapInput, LinearityInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import DarkImageProcessor
from pymetis.instruments.metis import qc


class MetisLssRsrfImpl(DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssRsrfRaw

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class LmRsrfWcuOffInput(RawInput):
            Item = WcuOffRaw

        raw: RawInput
        persistence_map: PersistenceMapInput
        gain_map: GainMapInput
        linearity: LinearityInput
        lm_rsrf_wcu_off: LmRsrfWcuOffInput

    class ProductSet(PipelineProductSet):
        MedianLssRsrf = MedianLssRsrf
        MeanLssRsrf = MeanLssRsrf
        MasterLssRsrf = MasterLssRsrf

    class Qc(QcParameterSet):
        MeanLevel = qc.lss.LssRsrfMeanLevel
        MedianLevel = qc.lss.LssRsrfMedianLevel
        InterorderLevel = qc.lss.LssRsrfInterorderLevel
        NormStdev = qc.lss.LssRsrfNormStdev
        NormSnr = qc.lss.LssRsrfNormSnr
    def process(self) -> set[DataItem]:
        """do something more fancy in the future"""
        # Load raw image
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        primary_header = create_dummy_header()

        # Final RSRF
        combined_master_hdr = create_dummy_header()
        combined_master_img = self.combine_images(raw_images, "median")

        # Mean combine
        combined_mean_hdr = create_dummy_header()
        combined_mean_img = self.combine_images(raw_images, "average")

        # Median combine
        combined_median_hdr = create_dummy_header()
        combined_median_img = self.combine_images(raw_images, "median")

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.InterorderLevel(None),
            self.Qc.MeanLevel(None),
            self.Qc.MedianLevel(None),
            self.Qc.NormSnr(None),
            self.Qc.NormStdev(None),
        ))

        return {
            self.ProductSet.MasterLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_master_hdr, combined_master_img, name='DET1.DATA'),
            ),
            self.ProductSet.MeanLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_mean_hdr, combined_mean_img, name='DET1.DATA'),
            ),
            self.ProductSet.MedianLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_median_hdr, combined_median_img, name='DET1.DATA'),
            ),
        }
