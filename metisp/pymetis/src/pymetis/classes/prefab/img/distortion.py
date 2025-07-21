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

import cpl
from cpl.core import Msg

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
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")
        table = self._create_dummy_table()

        return {
            self.ProductDistortionTable(self.header, table),
            self.ProductDistortionMap(self.header, combined_image),
            self.ProductDistortionReduced(self.header, table),
        }
