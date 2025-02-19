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
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.common import RawInput, MasterDarkInput, LinearityInput, PersistenceMapInput
from pymetis.inputs.mixins import PersistenceInputSetMixin, GainMapInputSetMixin

from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuReduceImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(GainMapInputSetMixin, PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_(?P<target>SCI|STD)_RAW")

        class RawSkyInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_SKY_RAW")
            _title: str = "blank sky image"

        class MasterDarkInput(MasterDarkInput):
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

        class WavecalInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_WAVECAL")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Wavelength calibration"

        class DistortionTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_DISTORTION_TABLE")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Distortion table"

        class RsrfInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"RSRF_IFU")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "RSRF"

        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            """
                Here we also define all input frames specific for this recipe, except those handled by mixins.
            """
            super().__init__(frameset)
            self.sky = self.RawSkyInput(frameset)
            self.linearity_map = LinearityInput(frameset)
            self.ifu_wavecal = self.WavecalInput(frameset)
            self.rsrf = self.RsrfInput(frameset)
            self.ifu_distortion_table = self.DistortionTableInput(frameset)

            self.inputs |= {self.sky, self.linearity_map, self.rsrf, self.ifu_wavecal, self.ifu_distortion_table}

    class ProductReduced(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED"

    class ProductBackground(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_BACKGROUND"

    class ProductReducedCube(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED_CUBE"

    class ProductCombined(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_COMBINED"

    def process_images(self) -> [PipelineProduct]:
        # do something... a lot of something

        self.target = self.inputset.tag_parameters["target"]

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        return [
            self.ProductReduced(self, header, image, target=self.target),
            self.ProductBackground(self, header, image, target=self.target),
            self.ProductReducedCube(self, header, image, target=self.target),
            self.ProductCombined(self, header, image, target=self.target),
        ]


class MetisIfuReduce(MetisRecipe):
    _name: str = "metis_ifu_reduce"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Reduce raw science exposures of the IFU."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuReduceImpl
