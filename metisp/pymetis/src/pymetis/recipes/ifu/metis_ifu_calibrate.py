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

from pymetis.classes.dataitems.common import TelluricCorrection, IfuScienceCubeCalibrated
from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.ifu.ifu import IfuSciReduced
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.products import PipelineImageProduct, PipelineProduct
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs.common import FluxCalTableInput


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciReducedInput(SinglePipelineInput):
            _item: type[DataItem] = IfuSciReduced
            _tags: re.Pattern = re.compile(r"IFU_SCI_REDUCED")

        class TelluricInput(SinglePipelineInput):
            _item: type[DataItem] = TelluricCorrection
            _tags: re.Pattern = re.compile(r"IFU_TELLURIC")

        FluxCalTableInput = FluxCalTableInput

    class ProductSciCubeCalibrated(PipelineImageProduct):
        _item = IfuScienceCubeCalibrated
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> set[PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product_scc = self.ProductSciCubeCalibrated(self, header, image)

        return {product_scc}


class MetisIfuCalibrate(MetisRecipe):
    _name: str = "metis_ifu_calibrate"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Calibrate IFU science data"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: set[str] = {'DRS.IFU'}
    _algorithm = """Correct for telluric absorption.
    Apply flux calibration."""

    implementation_class: type[MetisRecipeImpl] = MetisIfuCalibrateImpl
