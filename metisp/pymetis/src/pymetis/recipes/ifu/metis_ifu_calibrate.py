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
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.classes.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.classes.qc import QcParameterSet, QcParameter
from pymetis.dataitems.common import IfuTelluric
from pymetis.dataitems.ifu.ifu import IfuScienceCubeCalibrated, IfuSciReduced
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs.common import FluxCalTableInput
from pymetis.utils.dummy import create_dummy_image, create_dummy_header


class MetisIfuCalibrateImpl(BandIfuMixin, DetectorIfuMixin, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class ReducedInput(SinglePipelineInput):
            Item = IfuSciReduced

        class TelluricInput(SinglePipelineInput):
            Item = IfuTelluric

        FluxCalTableInput = FluxCalTableInput

    class ProductSet(PipelineProductSet):
        SciCubeCalibrated = IfuScienceCubeCalibrated

    class Qc(QcParameterSet):
        # QCs are apprently not very reusable, so we can define them here
        class MinFlux(QcParameter):
            _name_template = "QC IFU CALIB MINFLUX"
            _type = float
            _unit = "1"
            _description_template = "Minimum pixel flux in the calibrated image"
            _comment = None

        class MaxFlux(QcParameter):
            _name_template = "QC IFU CALIB MAXFLUX"
            _type = float
            _unit = "1"
            _description_template = "Maximum pixel flux in the calibrated image"
            _comment = None


    def process(self) -> set[DataItem]:
        reduced = self.inputset.reduced.load_data(extension='DET1.DATA')
        self.inputset.reduced.use()

        primary_header = create_dummy_header()
        header_scc = create_dummy_header()
        image = create_dummy_image()

        product_scc = self.ProductSet.SciCubeCalibrated(
            primary_header,
            Hdu(header_scc, image, name='IMAGE'),
        )

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

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisIfuCalibrateImpl
