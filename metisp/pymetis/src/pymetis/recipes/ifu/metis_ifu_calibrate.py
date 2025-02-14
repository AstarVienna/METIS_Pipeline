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
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciReducedInput(SinglePipelineInput):
            _title: str = "science reduced"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"IFU_SCI_REDUCED")

        class TelluricInput(SinglePipelineInput):
            _title: str = "telluric correction"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"IFU_TELLURIC")

        class FluxcalTabInput(SinglePipelineInput):
            _title: str = "flux calibration table"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"FLUXCAL_TAB")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.sci_reduced = self.SciReducedInput(frameset)
            self.telluric = self.TelluricInput(frameset)
            self.fluxcal = self.FluxcalTabInput(frameset)

            self.inputs |= {self.sci_reduced, self.telluric, self.fluxcal}

    class ProductSciCubeCalibrated(PipelineProduct):
        _tag = rf"IFU_SCI_CUBE_CALIBRATED"
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE


    def process_images(self) -> [PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product_scc = self.ProductSciCubeCalibrated(self, header, image)

        return [product_scc]


class MetisIfuCalibrate(MetisRecipe):
    _name: str = "metis_ifu_calibrate"
    _version: str = "0.1"
    _author: str = "Martin Baláž"
    _email: str = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Calibrate IFU science data"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    implementation_class =  MetisIfuCalibrateImpl
