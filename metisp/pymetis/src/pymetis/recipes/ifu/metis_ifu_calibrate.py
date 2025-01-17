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
        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.sci_reduced = SinglePipelineInput(frameset, tag=re.compile(r"IFU_SCI_REDUCED"))
            self.telluric = SinglePipelineInput(frameset, tag=re.compile(r"IFU_TELLURIC"))
            self.fluxcal = SinglePipelineInput(frameset, tag=re.compile(r"FLUXCAL_TAB"))

            self.inputs += [self.sci_reduced, self.telluric, self.fluxcal]

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        self.products = {
            product.category: product()
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products


class MetisIfuCalibrate(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Calibrate IFU science data"
    _description = (
        "Currently just a skeleton prototype."
    )

    def dispatch_implementation_class(self, frameset: cpl.ui.FrameSet) -> type[MetisRecipeImpl]:
        return MetisIfuCalibrateImpl
