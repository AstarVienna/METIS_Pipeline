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

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.background.subtracted import StdBackgroundSubtracted
from pymetis.dataitems.combined import Combined
from pymetis.dataitems.common import FluxCalTable
from pymetis.classes.inputs import RawInput
from pymetis.classes.inputs import FluxstdCatalogInput
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.utils.dummy import create_dummy_table


class MetisImgStdProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = StdBackgroundSubtracted

        FluxstdCatalogInput = FluxstdCatalogInput

    ProductImgFluxCalTable = FluxCalTable
    ProductImgStdCombined = Combined

    def process(self) -> set[DataItem]:
        raw_images = self.inputset.raw.load_images(extension=0)

        combined_image = self.combine_images(raw_images, "average")
        table = create_dummy_table()

        product_fluxcal = self.ProductImgFluxCalTable(self.header, table)
        product_combined = self.ProductImgStdCombined(self.header, combined_image)

        return {product_fluxcal, product_combined}
