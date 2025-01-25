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
from typing import Dict, Literal

from pymetis.base import MetisRecipe
from pymetis.base.product import PipelineProduct, TargetSpecificProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.common import RawInput, MasterDarkInput, LinearityInput, PersistenceMapInput

from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuReduceImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags = re.compile(r"IFU_(?P<target>SCI|STD)_RAW")

        class MasterDarkInput(MasterDarkInput):
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

        class WavecalInput(SinglePipelineInput):
            _tags = re.compile(r"IFU_WAVECAL")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Wavelength calibration"

        class DistortionTableInput(SinglePipelineInput):
            _tags = re.compile(r"IFU_DISTORTION_TABLE")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Distortion table"

        def __init__(self, frameset: cpl.ui.FrameSet):
            """
                Here we also define all input frames specific for this recipe, except those handled by mixins.
            """
            super().__init__(frameset)
            self.raw = self.RawInput(frameset)
            self.linearity_map = LinearityInput(frameset)
            self.persistence_map = PersistenceMapInput(frameset)
            self.master_dark = self.MasterDarkInput(frameset)
            self.ifu_wavecal = self.WavecalInput(frameset)
            self.ifu_distortion_table = self.DistortionTableInput(frameset)
            self.inputs += [self.linearity_map, self.persistence_map, self.master_dark, self.ifu_wavecal, self.ifu_distortion_table]

    class ProductReduced(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED"

    class ProductBackground(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_BACKGROUND"

    class ProductReducedCube(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED_CUBE"

    class ProductCombined(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_COMBINED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # do something... a lot of something

        print(self.inputset.tag_parameters)
        self.target = self.inputset.tag_parameters["target"]

        header = cpl.core.PropertyList()
        images = self.load_raw_images()
        image = self.combine_images(images, "add")

        self.products = {
            rf'IFU_{self.target}_REDUCED': self.ProductReduced(self, header, image, target=self.target),
            rf'IFU_{self.target}_BACKGROUND': self.ProductBackground(self, header, image, target=self.target),
            rf'IFU_{self.target}_REDUCED_CUBE': self.ProductReducedCube(self, header, image, target=self.target),
            rf'IFU_{self.target}_COMBINED': self.ProductCombined(self, header, image, target=self.target),
        }
        return self.products


class MetisIfuReduce(MetisRecipe):
    _name = "metis_ifu_reduce"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuReduceImpl

    # This should not be here but without it pyesorex crashes
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterValue(
            name=f"{_name}.dummy",
            context=_name,
            description="Dummy parameter",
            default="dummy",
        )
    ])
