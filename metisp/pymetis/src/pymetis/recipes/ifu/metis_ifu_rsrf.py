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

import cpl
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet, MultiplePipelineInput, BadpixMapInput, \
                           MasterDarkInput, LinearityInput, GainMapInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class RawInput(MultiplePipelineInput):
    _tags = ["IFU_RSRF_RAW"]


class RsrfMasterDarkInput(MasterDarkInput):
    pass


class DistortionTableInput(SinglePipelineInput):
    _tags = ["IFU_DISTORTION_TABLE"]


class WavecalInput(SinglePipelineInput):
    _tags = ["IFU_WAVECAL"]


class MetisIfuRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector = '2RG'

        RawInput = RawInput
        MasterDarkInput = RsrfMasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.badpix_map = BadpixMapInput(frameset, required=False)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)
            self.wavecal = WavecalInput(frameset)

            self.inputs += [self.badpix_map, self.linearity, self.gain_map]

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


class MetisIfuRsrf(MetisRecipe):
    _name = "metis_ifu_rsrf"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Determine the relative spectral response function"
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([])

    implementation_class = MetisIfuRsrfImpl
