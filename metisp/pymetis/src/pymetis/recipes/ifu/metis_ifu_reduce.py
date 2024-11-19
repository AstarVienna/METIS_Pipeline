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
from cpl.core import Msg
from typing import Any, Dict, Literal

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.common import RawInput, MasterDarkInput, LinearityInput, PersistenceMapInput
from pymetis.mixins import DetectorIfuMixin

from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuReduceImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(DarkImageProcessor.InputSet):
        """
            The Input class for Metis IFU reduction. Utilizes InputMixins:

            - Detector2rgMixin, which handles the 2RG detector and substitudes '2RG' for 'det' in tags
            - LinearityInputMixin, which
        """
        detector = "IFU"

        class RawInput(RawInput):
            _tags = ["IFU_SCI_RAW", "IFU_STD_RAW"]

        class MasterDarkInput(DetectorIfuMixin, MasterDarkInput):
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

        class WavecalInput(SinglePipelineInput):
            _tags = ["IFU_WAVECAL"]
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Wavelength calibration"

        class DistortionTableInput(SinglePipelineInput):
            _tags = ["IFU_DISTORTION_TABLE"]
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "Distortion table"

        def __init__(self, frameset: cpl.ui.FrameSet):
            """
                Here we also define all input frames specific for this recipe, except those handled by mixins.
            """
            self.raw = self.RawInput(frameset, det=self.detector)
            self.linearity_map = LinearityInput(frameset, det=self.detector)
            self.persistence_map = PersistenceMapInput(frameset)
            self.master_dark = self.MasterDarkInput(frameset, det="IFU")
            self.ifu_wavecal = self.WavecalInput(frameset)
            self.ifu_distortion_table = self.DistortionTableInput(frameset)
            self.inputs += [self.linearity_map, self.persistence_map, self.master_dark, self.ifu_wavecal, self.ifu_distortion_table]
            super().__init__(frameset)

    class ProductReduced(PipelineProduct):
        target = "SCI"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED"

    class ProductBackground(PipelineProduct):
        target = "SCI"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_BACKGROUND"

    class ProductReducedCube(PipelineProduct):
        target = "SCI"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_REDUCED_CUBE"

    class ProductCombined(PipelineProduct):
        target = "SCI"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_{self.target}_COMBINED"


    def process_images(self) -> Dict[str, PipelineProduct]:
        # do something... a lot of something

        header = cpl.core.PropertyList()
        images = self.load_raw_images()
        image = self.combine_images(images, "add")

        self.products = {
            rf'IFU_{self.target}_REDUCED': self.ProductReduced(self, header, image),
            rf'IFU_{self.target}_BACKGROUND': self.ProductBackground(self, header, image),
            rf'IFU_{self.target}_REDUCED_CUBE': self.ProductReducedCube(self, header, image),
            rf'IFU_{self.target}_COMBINED': self.ProductCombined(self, header, image),
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

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_reduce.telluric",
            context="metis_ifu_reduce",
            description="Use telluric correction",
            default=False,
            alternatives=(True, False),
        ),
    ])
    implementation_class = MetisIfuReduceImpl
