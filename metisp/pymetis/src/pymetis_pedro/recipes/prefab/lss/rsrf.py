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

import cpl

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.lss.rsrf import LssRsrfRaw, MedianLssRsrf, MeanLssRsrf, MasterLssRsrf
from pymetis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.inputs import RawInput, PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, \
    LinearityInputSetMixin
from pymetis.recipes.prefab import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header


class MetisLssRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssRsrfRaw

        class LmRsrfWcuOffInput(RawInput):
            Item = WcuOffRaw

    ProductMedianLssRsrf = MedianLssRsrf
    ProductMeanLssRsrf = MeanLssRsrf
    ProductMasterLssRsrf = MasterLssRsrf


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

        return {
            self.ProductMasterLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_master_hdr, combined_master_img, name='DET1.DATA'),
            ),
            self.ProductMeanLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_mean_hdr, combined_mean_img, name='DET1.DATA'),
            ),
            self.ProductMedianLssRsrf(
                copy.deepcopy(primary_header),
                Hdu(combined_median_hdr, combined_median_img, name='DET1.DATA'),
            ),
        }
