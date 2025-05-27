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
from abc import ABC

import cpl

from pymetis.classes.products import BandSpecificProduct, PipelineImageProduct
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.classes.products.product import PipelineProduct
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs import FluxcalTableInput


class MetisImgCalibrateImpl(MetisRecipeImpl, ABC):
    class InputSet(PipelineInputSet):
        class BackgroundInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"(?P<band>LM|N)_SCI_BKG_SUBTRACTED")
            _title: str = "science background-subtracted"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

            @classmethod
            def description(cls):
                return rf"Thermal background subtracted images of science {cls._band} exposures."

        FluxcalTableInput = FluxcalTableInput

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            _title: str = "distortion table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description: str = "Table of distortion information"

    class ProductSciCalibrated(BandSpecificProduct, PipelineImageProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        group = cpl.ui.Frame.FrameGroup.CALIB   # ToDO Review if this should not be PRODUCT instead.
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def tag(cls) -> str:
            return rf"{cls.band()}_SCI_CALIBRATED"

        @classmethod
        def description(cls) -> str:
            return rf"{cls.band()} band image with flux calibration, WC coordinate system and distorion information"

    def process_images(self) -> set[PipelineProduct]:
        combined_image = self._create_dummy_image()
        product_calibrated: PipelineProduct = self.ProductSciCalibrated(self, self.header, combined_image)

        return {product_calibrated}
