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
from pymetis.dataitems.background.subtracted import SciBackgroundSubtracted
from pymetis.dataitems.distortion.table import DistortionTable
from pymetis.dataitems.img.basicreduced import Calibrated
from pymetis.classes.inputs import FluxCalTableInput
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.utils.dummy import create_dummy_header


class MetisImgCalibrateImpl(MetisRecipeImpl, ABC):
    class InputSet(PipelineInputSet, abstract=True):
        class BackgroundInput(SinglePipelineInput):
            Item = SciBackgroundSubtracted

        FluxcalTableInput = FluxCalTableInput

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            Item = DistortionTable

    ProductSciCalibrated = Calibrated

    def process(self) -> set[DataItem]:
        background = self.inputset.background.load_data('PRIMARY')
        primary_header = self.inputset.background.item.primary_header
        header = create_dummy_header()

        product_calibrated = self.ProductSciCalibrated(
            primary_header,
            Hdu(header, background, name='PRIMARY'),
        )

        return {product_calibrated}
