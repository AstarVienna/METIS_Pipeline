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
from pymetis.inputs import SinglePipelineInput, MultiplePipelineInput, BadpixMapInput, \
                           MasterDarkInput, LinearityInput, RawInput, GainMapInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class RsrfMasterDarkInput(MasterDarkInput):
    pass


class DistortionTableInput(SinglePipelineInput):
    _tags = re.compile(r"IFU_DISTORTION_TABLE")
    _title = "distortion table"
    _group = cpl.ui.Frame.FrameGroup.CALIB


class MetisIfuRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"IFU_RSRF_RAW")
            _title = "IFU rsrf raw"

        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)

            self.inputs += [self.gain_map, self.distortion_table]

    class ProductMasterFlatIfu(PipelineProduct):
        category = rf"MASTER_FLAT_IFU"
        tag = category
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    class ProductRsrfIfu(PipelineProduct):
        category = rf"RSRF_IFU"
        tag = category
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    class ProductBadpixMapIfu(PipelineProduct):
        category = rf"BADPIX_MAP_IFU"
        tag = category
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        self.products = {
            product.category: product(self, header, image)
            for product in [self.ProductMasterFlatIfu, self.ProductRsrfIfu, self.ProductBadpixMapIfu]
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

    # This should not be here but without it pyesorex crashes
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_rsrf.telluric",
            context="metis_ifu_rsrf",
            description="Use telluric correction",
            default=False,
            alternatives=(True, False),
        ),
    ])
    implementation_class = MetisIfuRsrfImpl
