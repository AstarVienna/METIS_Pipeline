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

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.distortion.map import DistortionMap
from pymetis.dataitems.distortion.raw import DistortionRaw
from pymetis.dataitems.distortion.reduced import DistortionReduced
from pymetis.dataitems.distortion.table import DistortionTable
from pymetis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import (RawInput, SinglePipelineInput, PinholeTableInput, PersistenceInputSetMixin,
                                    LinearityInputSetMixin, GainMapInputSetMixin)
from pymetis.utils.dummy import create_dummy_table, create_dummy_image, create_dummy_header


class MetisBaseImgDistortionImpl(RawImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = WcuOffRaw

        class DistortionInput(SinglePipelineInput):
            Item = DistortionRaw

        class PinholeTableInput(PinholeTableInput):
            pass

    ProductDistortionTable = DistortionTable
    ProductDistortionMap = DistortionMap
    ProductDistortionReduced = DistortionReduced

    def process(self) -> set[DataItem]:
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        combined_image = self.combine_images(raw_images, "average")
        distortion = self.inputset.distortion.load_data('DET1.DATA')
        primary_header = self.inputset.distortion.item.primary_header

        header_distortion_table = create_dummy_header()
        header_distortion_map = create_dummy_header()
        header_distortion_reduced = create_dummy_header()
        table = create_dummy_table()
        image = create_dummy_image()

        return {
            self.ProductDistortionTable(
                primary_header,
                Hdu(header_distortion_table, table, name='TABLE'),
            ),
            self.ProductDistortionMap(
                primary_header,
                Hdu(header_distortion_map, combined_image, name='DET1.DATA'),
            ),
            self.ProductDistortionReduced(
                primary_header,
                Hdu(header_distortion_reduced, image, name='IMAGE'),
            ),
        }
