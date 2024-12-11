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

from pymetis.base import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.common import BadpixMapInput, MasterDarkInput, LinearityInput, GainMapInput, RawInput, DistortionTableInput, WavecalInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuWavecalImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"IFU_WAVE_RAW")

        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)

            self.inputs += [self.gain_map, self.distortion_table]

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"
        tag = category
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = cpl.core.PropertyList()
        images = self.load_raw_images()
        image = self.combine_images(images, "add")

        self.products = {
            product.category: product(self, header, image)
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products


class MetisIfuWavecal(MetisRecipe):
    _name = "metis_ifu_wavecal"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Determine the relative spectral response function"
    _description = (
        "Currently just a skeleton prototype."
    )

    # This should not be here but without it pyesorex crashes
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_reduce.telluric",
            context="metis_ifu_reduce",
            description="Use telluric correction",
            default=False,
            alternatives=(True, False),
        ),
    ])
    implementation_class = MetisIfuWavecalImpl
