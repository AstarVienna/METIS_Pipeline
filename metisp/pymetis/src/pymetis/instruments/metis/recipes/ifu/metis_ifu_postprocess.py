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

from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.engine.core.parameter import ParameterList, ParameterEnum
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import PipelineInputSet, MultiplePipelineInput, PrimaryInputMixin
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe

from pymetis.instruments.metis.dataitems.coadd import IfuSciCoadd
from pymetis.instruments.metis.dataitems.ifu.ifu import IfuScienceCubeCalibrated
from pymetis.instruments.metis.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis import qc


class MetisIfuPostprocessImpl(BandIfuMixin, DetectorIfuMixin, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciCubeCalibratedInput(PrimaryInputMixin, MultiplePipelineInput):
            Item = IfuScienceCubeCalibrated

        sci_cube_calibrated: SciCubeCalibratedInput

    class ProductSet(PipelineProductSet):
        SciCoadd = IfuSciCoadd

    class Qc(QcParameterSet):
        # QCs are apprently not very reusable, so we can define them here
        GridRange = qc.ifu.IfuPostprocGridRange
        MedMean = qc.ifu.IfuPostprocMedMean
        MedRms = qc.ifu.IfuPostprocMedRms
        MedMed = qc.ifu.IfuPostprocMedMed
        DeltaC = qc.ifu.IfuPostprocDeltaC
    def determine_output_grid(self):
        pass

    def resample_cubes(self):
        pass

    def coadd_cubes(self):
        calibrated = self.inputset.sci_cube_calibrated.load_data('IMAGE')
        coadded = calibrated.collapse_create()
        return coadded

    def process(self) -> set[DataItem]:
        self.determine_output_grid()
        self.resample_cubes()
        self.coadd_cubes()

        primary_header = create_dummy_header()
        header_coadd = create_dummy_header()

        coadded = self.coadd_cubes()

        product = self.ProductSet.SciCoadd(
            primary_header,
            Hdu(header_coadd, coadded, name='IMAGE')
        )

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.DeltaC(None),
            self.Qc.GridRange(None),
            self.Qc.MedMean(None),
            self.Qc.MedMed(None),
            self.Qc.MedRms(None),
        ))

        return {product}  # ToDo is just a dummy for now


class MetisIfuPostprocess(Recipe):
    _name: str = "metis_ifu_postprocess"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Coaddition and mosaicing of reduced science cubes."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: frozenset[str] = frozenset({'DRS.IFU'})
    _algorithm = """Call metis_ifu_grid_output to find the output grid encompassing all input cubes
    Call metis_ifu_resampling to resample input cubes to output grid
    Call metis_ifu_coadd to stack the images"""

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

    Impl = MetisIfuPostprocessImpl
