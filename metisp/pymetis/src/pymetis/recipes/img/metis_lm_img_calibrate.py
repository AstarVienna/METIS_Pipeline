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

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet


class MetisLmImgCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class BackgroundInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_SCI_BKG_SUBTRACTED")
            _title: str = "science background-subtracted"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        class PinholeTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"FLUXCAL_TAB")
            _title: str = "flux table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_DISTORTION_TABLE")
            _title: str = "distortion table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.background = self.BackgroundInput(frameset)
            self.flux_table = self.PinholeTableInput(frameset)
            self.distortion_table = self.DistortionTableInput(frameset)
            self.inputs |= {self.background, self.flux_table, self.distortion_table}

    class ProductLmSciCalibrated(PipelineProduct):
        _tag = r"LM_SCI_CALIBRATED"
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> [PipelineProduct]:
        combined_image = self._create_dummy_image()
        product_calibrated = self.ProductLmSciCalibrated(self, self.header, combined_image)

        return [product_calibrated]


class MetisLmImgCalibrate(MetisRecipe):
    _name = "metis_lm_img_calibrate"
    _version = "0.1"
    _author = "Chi-Hung Yan"
    _email = "chyan@asiaa.sinica.edu.tw"
    _synopsis = "Determine optical distortion coefficients for the LM imager."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgCalibrateImpl
