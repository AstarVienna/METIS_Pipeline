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

from pymetis.base import MetisRecipeImpl
from pymetis.products.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.inputs.common import FluxcalTableInput


class MetisImgCalibrateImpl(MetisRecipeImpl, ABC):
    class InputSet(PipelineInputSet):
        class BackgroundInput(SinglePipelineInput):
            _title: str = "science background-subtracted"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description: str = "Thermal background subtracted images of science LM exposures."

        FluxcalTableInput = FluxcalTableInput

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            _title: str = "distortion table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description: str = "Table of distortion information"

    class ProductSciCalibrated(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        group = cpl.ui.Frame.FrameGroup.CALIB
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    def process_images(self) -> [PipelineProduct]:
        combined_image = self._create_dummy_image()
        product_calibrated = self.ProductSciCalibrated(self, self.header, combined_image)

        return [product_calibrated]