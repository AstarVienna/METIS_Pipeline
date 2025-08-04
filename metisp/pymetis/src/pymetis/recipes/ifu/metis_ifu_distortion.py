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
from cpl.core import Msg

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.dataitems.distortion import IfuDistortionRaw, IfuDistortionTable, IfuDistortionReduced
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import RawInput, MasterDarkInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_table


class MetisIfuDistortionImpl(DarkImageProcessor):
    class InputSet(LinearityInputSetMixin, GainMapInputSetMixin, PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        MasterDarkInput = MasterDarkInput
        PinholeTableInput = PinholeTableInput

        class RawInput(RawInput):
            Item = IfuDistortionRaw

    ProductDistortionTable = IfuDistortionTable
    ProductDistortionReduced = IfuDistortionReduced

    def process(self) -> set[DataItem]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")
        table = create_dummy_table()

        product_distortion = self.ProductDistortionTable(self.header, table)
        product_distortion_reduced = self.ProductDistortionReduced(self.header, combined_image)

        return {product_distortion, product_distortion_reduced}


class MetisIfuDistortion(MetisRecipe):
    _name = "metis_ifu_distortion"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Calculate table mapping pixel position to position on sky."""

    Impl = MetisIfuDistortionImpl
