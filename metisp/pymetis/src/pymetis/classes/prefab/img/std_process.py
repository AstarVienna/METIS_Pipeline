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

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.background.subtracted import StdBackgroundSubtracted
from pymetis.dataitems.combined import Combined
from pymetis.dataitems.common import FluxCalTable
from pymetis.classes.inputs import RawInput
from pymetis.classes.inputs import FluxstdCatalogInput
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.utils.dummy import create_dummy_table, create_dummy_header


class MetisImgStdProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = StdBackgroundSubtracted

        FluxstdCatalogInput = FluxstdCatalogInput

    ProductImgFluxCalTable = FluxCalTable
    ProductImgStdCombined = Combined

    def process(self) -> set[DataItem]:
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        combined_image = self.combine_images(raw_images, "average")
        primary_header = self.inputset.raw.items[0].primary_header

        header_table = create_dummy_header()
        header_combined = create_dummy_header()
        table = create_dummy_table()

        product_fluxcal = self.ProductImgFluxCalTable(
            copy.deepcopy(primary_header),
            Hdu(header_table, table, name='TABLE')
        )
        product_combined = self.ProductImgStdCombined(
            copy.deepcopy(primary_header),
            Hdu(header_combined, combined_image, name='IMAGE'),
        )

        return {product_fluxcal, product_combined}
