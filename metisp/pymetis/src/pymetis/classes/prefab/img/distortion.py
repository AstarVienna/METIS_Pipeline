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

from abc import ABC

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.distortion.map import DistortionMap
from pymetis.dataitems.distortion.raw import DistortionRaw
from pymetis.dataitems.distortion.reduced import DistortionReduced
from pymetis.dataitems.distortion.table import DistortionTable
from pymetis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import RawInput, SinglePipelineInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.utils.dummy import create_dummy_table, create_dummy_image


class MetisBaseImgDistortionImpl(RawImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = WcuOffRaw

        class DistortionInput(SinglePipelineInput):
            Item = DistortionRaw

        PinholeTableInput = PinholeTableInput

    ProductDistortionTable = DistortionTable
    ProductDistortionMap = DistortionMap
    ProductDistortionReduced = DistortionReduced

    def process(self) -> set[DataItem]:
        combined_image = self.combine_images(self.inputset.load_raw_images(), "average")
        distortion = self.inputset.distortion.load_data().use()

        header = distortion.header
        table = create_dummy_table()
        image = create_dummy_image()

        return {
            self.ProductDistortionTable(header, table),
            self.ProductDistortionMap(header, combined_image),
            self.ProductDistortionReduced(header, image),
        }
