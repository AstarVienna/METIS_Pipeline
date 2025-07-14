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

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.common import IfuTelluric
from pymetis.classes.dataitems.ifu.ifu import IfuScienceCubeCalibrated, IfuSciReduced
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs.common import FluxCalTableInput


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class ReducedInput(SinglePipelineInput):
            Item = IfuSciReduced

        class TelluricInput(SinglePipelineInput):
            Item = IfuTelluric

        FluxCalTableInput = FluxCalTableInput

    ProductSciCubeCalibrated = IfuScienceCubeCalibrated

    def process(self) -> set[DataItem]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product_scc = self.ProductSciCubeCalibrated(header, image)

        return {product_scc}


class MetisIfuCalibrate(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Calibrate IFU science data"
    _description = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Correct for telluric absorption.
    Apply flux calibration."""

    Impl = MetisIfuCalibrateImpl
