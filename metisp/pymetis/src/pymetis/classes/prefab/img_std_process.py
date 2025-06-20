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
from cpl.core import Msg

from pymetis.classes.products import PipelineProduct, BandSpecificProduct, PipelineTableProduct, PipelineImageProduct
from pymetis.classes.inputs import RawInput
from pymetis.classes.inputs import FluxstdCatalogInput
from pymetis.classes.prefab.rawimage import RawImageProcessor


class MetisImgStdProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"(?P<band>LM|N)_STD_BKG_SUBTRACTED")
            _description: str = "Thermal background subtracted images of standard LM/N exposures."

        FluxstdCatalogInput = FluxstdCatalogInput

    class ProductImgFluxCalTable(PipelineTableProduct):
        _tag = r"FLUXCAL_TAB"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE
        _description: str = "Conversion between instrumental and physical flux units."
        _oca_keywords = {'PRO.CATG'}

    class ProductImgStdCombined(BandSpecificProduct, PipelineImageProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def description(cls) -> str:
            return f"Stacked {cls.band()} band exposures."

        @classmethod
        def tag(cls) -> str:
            return fr"{cls.band()}_STD_COMBINED"

    def process_images(self) -> set[PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=0)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")
        table = self._create_dummy_table()

        product_fluxcal = self.ProductImgFluxCalTable(self, self.header, table)
        product_combined = self.ProductImgStdCombined(self, self.header, combined_image)

        return {product_fluxcal, product_combined}
