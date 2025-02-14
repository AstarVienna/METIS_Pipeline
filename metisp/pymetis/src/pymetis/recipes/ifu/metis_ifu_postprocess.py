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
from cpl.core import Msg
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.base.product import PipelineProduct


class MetisIfuPostprocessImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciCubeCalibratedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_SCI_CUBE_CALIBRATED")
            _title: str = "rectified spectral cube"
            _group = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.sci_cube_calibrated = self.SciCubeCalibratedInput(frameset)
            self.inputs |= {self.sci_cube_calibrated}

    class ProductSciCoadd(PipelineProduct):
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_SCI_COADD"

    def determine_output_grid(self):
        pass

    def resample_cubes(self):
        pass

    def coadd_cubes(self):
        pass

    def process_images(self) -> [PipelineProduct]:
        self.determine_output_grid()
        self.resample_cubes()
        self.coadd_cubes()

        header = self._create_dummy_header()
        image = cpl.core.Image.load(self.inputset.sci_cube_calibrated.frame.file) # ToDo actual processing

        product = self.ProductSciCoadd(self, header, image)

        return [product] # ToDo is just a dummy for now


class MetisIfuPostprocess(MetisRecipe):
    _name: str = "metis_ifu_postprocess"
    _version: str = "0.1"
    _author: str = "Martin Baláž"
    _email: str = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis: str = "Calibrate IFU science data"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuPostprocessImpl

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.telluric",
            context=_name,
            description="Use telluric correction",
            default=False,
            alternatives=(True, False),
        ),
    ])
