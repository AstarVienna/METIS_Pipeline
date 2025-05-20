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

from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import RawInput, SinglePipelineInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.products import PipelineTableProduct, PipelineImageProduct
from pymetis.classes.products.product import PipelineProduct
from pymetis.classes.products.common import BandSpecificProduct


class MetisBaseImgDistortionImpl(RawImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _description: str = "Raw data for dark subtraction in other recipes."

        class DistortionInput(SinglePipelineInput):
            _title: str = "Distortion map"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description: str = "Images of grid mask in WCU-FP2 or CFO-FP2."

        PinholeTableInput = PinholeTableInput

    class ProductDistortionTable(BandSpecificProduct, PipelineTableProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Table of distortion information"
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def tag(cls) -> str:
            return rf"{cls.band():s}_DISTORTION_TABLE"

    class ProductDistortionMap(BandSpecificProduct, PipelineImageProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Map of pixel scale across the detector"
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def tag(cls) -> str:
            return rf"{cls.band():s}_DISTORTION_MAP"

    class ProductDistortionReduced(BandSpecificProduct, PipelineTableProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Table of polynomial coefficients for distortion correction"
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def tag(cls) -> str:
            return rf"{cls.band():s}_DIST_REDUCED"

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")
        table = self._create_dummy_table()

        return [
            self.ProductDistortionTable(self, self.header, table),
            self.ProductDistortionMap(self, self.header, combined_image),
            self.ProductDistortionReduced(self, self.header, table),
        ]
