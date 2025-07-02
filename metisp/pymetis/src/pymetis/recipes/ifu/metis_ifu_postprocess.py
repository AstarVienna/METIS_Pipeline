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

from pymetis.classes.dataitems.coadd import IfuSciCoadd
from pymetis.classes.dataitems.common import IfuScienceCubeCalibrated
from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.products import PipelineProduct, PipelineImageProduct


class MetisIfuPostprocessImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciCubeCalibratedInput(SinglePipelineInput):
            Item: type[DataItem] = IfuScienceCubeCalibrated
            _tags: re.Pattern = re.compile(r"IFU_SCI_CUBE_CALIBRATED")

    ProductSciCoadd = IfuSciCoadd

    def determine_output_grid(self):
        pass

    def resample_cubes(self):
        pass

    def coadd_cubes(self):
        pass

    def process_images(self) -> set[PipelineProduct]:
        self.determine_output_grid()
        self.resample_cubes()
        self.coadd_cubes()

        header = self._create_dummy_header()
        image = cpl.core.Image.load(self.inputset.sci_cube_calibrated.frame.file)  # ToDo actual processing

        product = self.ProductSciCoadd(header, image)

        return {product}  # ToDo is just a dummy for now


class MetisIfuPostprocess(MetisRecipe):
    _name: str = "metis_ifu_postprocess"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Coaddition and mosaicing of reduced science cubes."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Call metis_ifu_grid_output to find the output grid encompassing all input cubes
    Call metis_ifu_resampling to resample input cubes to output grid
    Call metis_ifu_coadd to stack the images"""

    implementation_class = MetisIfuPostprocessImpl
