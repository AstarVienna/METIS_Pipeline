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

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.products import PipelineProduct, PipelineImageProduct, PipelineTableProduct
from pymetis.classes.inputs import RawInput, MasterDarkInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.prefab.darkimage import DarkImageProcessor


class MetisIfuDistortionImpl(DarkImageProcessor):
    class InputSet(LinearityInputSetMixin, GainMapInputSetMixin, PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        MasterDarkInput = MasterDarkInput
        PinholeTableInput = PinholeTableInput

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_DISTORTION_RAW")
            _description: str = "Images of multi-pinhole mask."

    class ProductIfuDistortionTable(PipelineTableProduct):
        _tag = r"IFU_DISTORTION_TABLE"
        level = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Table of distortion coefficients for an IFU data set"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

    class ProductIfuDistortionReduced(PipelineImageProduct):
        _tag = r"IFU_DIST_REDUCED"
        level = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Table of polynomial coefficients for distortion correction"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

    def process_images(self) -> list[PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")
        table = self._create_dummy_table()

        product_distortion = self.ProductIfuDistortionTable(self, self.header, table)
        product_distortion_reduced = self.ProductIfuDistortionReduced(self, self.header, combined_image)

        return [product_distortion, product_distortion_reduced]


class MetisIfuDistortion(MetisRecipe):
    _name: str = "metis_ifu_distortion"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Reduce raw science exposures of the IFU."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: {str} = {'DRS.IFU'}
    _algorithm = """Calculate table mapping pixel position to position on sky."""

    implementation_class = MetisIfuDistortionImpl
