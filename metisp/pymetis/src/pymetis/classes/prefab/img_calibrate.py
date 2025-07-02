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

from pymetis.classes.dataitems.background.subtracted import LmSciBackgroundSubtracted
from pymetis.classes.dataitems.img.basicreduced import LmSciCalibrated
from pymetis.classes.dataitems.distortion.table import DistortionTable
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs import FluxCalTableInput


class MetisImgCalibrateImpl(MetisRecipeImpl, ABC):
    class InputSet(PipelineInputSet):
        class BackgroundInput(SinglePipelineInput):
            Item = LmSciBackgroundSubtracted

        FluxcalTableInput = FluxCalTableInput

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            Item = DistortionTable

    ProductSciCalibrated = LmSciCalibrated

    def process_images(self):
        combined_image = self._create_dummy_image()
        product_calibrated = self.ProductSciCalibrated(self.header, combined_image)

        return {product_calibrated}
