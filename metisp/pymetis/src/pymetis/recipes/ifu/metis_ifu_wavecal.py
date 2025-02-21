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
from pymetis.inputs.common import MasterDarkInput, GainMapInput, RawInput, DistortionTableInput, LinearityInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuWavecalImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_WAVE_RAW")

        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)
            self.linearity = LinearityInput(frameset)

            self.inputs |= {self.gain_map, self.distortion_table, self.linearity}

    class ProductIfuWavecal(PipelineProduct):
        _tag = r"IFU_WAVECAL"
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> [PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        product_wavecal = self.ProductIfuWavecal(self, header, image)

        return [product_wavecal]


class MetisIfuWavecal(MetisRecipe):
    _name: str = "metis_ifu_wavecal"
    _version: str = "0.1"
    _author: str = "Martin Baláž"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Determine the relative spectral response function"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuWavecalImpl
