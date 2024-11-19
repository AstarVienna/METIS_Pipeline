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
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.base.product import PipelineProduct
from pymetis.inputs.mixins import DetectorIfuMixin


class MetisIfuPostprocessImpl(MetisRecipeImpl):
    class InputSet(DetectorIfuMixin, PipelineInputSet):
        class SciCubeCalibratedInput(SinglePipelineInput):
            _tags = [r"IFU_SCI_CUBE_CALIBRATED"]
            _title = "rectified spectral cube"
            _group = cpl.ui.Frame.FrameGroup.CALIB

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.sci_cube_calibrated = self.SciCubeCalibratedInput(frameset)
            self.inputs += [self.sci_cube_calibrated]
            super().__init__(frameset)

    class ProductSciCoadd(PipelineProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"IFU_SCI_COADD"

    def determine_output_grid(self):
        pass

    def resample_cubes(self):
        pass

    def coadd_cubes(self):
        pass

    def process_images(self) -> Dict[str, PipelineProduct]:
        self.determine_output_grid()
        self.resample_cubes()
        self.coadd_cubes()

        header = cpl.core.PropertyList()
        coadded_image = cpl.core.Image.load(self.inputset.sci_cube_calibrated.frame.file) # ToDo actual processing
        print(self.inputset.sci_cube_calibrated.frame.file)

        self.products = {
            r'IFU_SCI_COADD': self.ProductSciCoadd(self, header, coadded_image),
        }
        return self.products


class MetisIfuPostprocess(MetisRecipe):
    _name = "metis_ifu_postprocess"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Calibrate IFU science data"
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
    implementation_class = MetisIfuPostprocessImpl
