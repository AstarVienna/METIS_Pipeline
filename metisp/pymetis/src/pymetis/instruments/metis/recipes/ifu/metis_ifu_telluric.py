"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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

from pymetis.engine.core.parameter import ParameterList, ParameterEnum

from pymetis.engine.recipes import Recipe
from pymetis.engine.inputs import SinglePipelineInput, PipelineInputSet, PrimaryInputMixin
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_image, create_dummy_table

from pymetis.instruments.metis.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.instruments.metis.dataitems.common import FluxCalTable
from pymetis.instruments.metis.dataitems.ifu.ifu import IfuReduced1d, IfuCombined, IfuTelluric
from pymetis.instruments.metis.inputs import FluxstdCatalogInput, LsfKernelInput, AtmProfileInput
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis import qc


# The aim of this recipe is twofold:
#   (a) to determine the transmission function for telluric absorption correction
#   (b) determination of the response function for the flux calibration
#
# Note that there will be most probably a redesign / split into more recipes to follow the approach
# implemented already in other ESO pipelines

class MetisIfuTelluricImpl(DetectorIfuMixin, BandIfuMixin, MetisRecipeImpl):
    """Implementation class for metis_ifu_telluric"""

    # ++++++++++++++ Defining input +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class InputSet(PipelineInputSet):
        """Inputs for metis_ifu_telluric"""
        # TODO the molecfit input proper is the uncorrected 1D spectrum (IFU_{target}_REDUCED_1D);
        #  until the recipe consumes it, the combined 2D product stands in as the primary input.

        class CombinedInput(PrimaryInputMixin, SinglePipelineInput):
            Item = IfuCombined

        combined: CombinedInput
        fluxstd_catalog: FluxstdCatalogInput
        lsf_kernel: LsfKernelInput
        atm_profile: AtmProfileInput

    # ++++++++++++++ Defining ouput +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Recipe is foreseen to do both, create transmission and response functions
    # We therefore need to define transmission spectrum and response curve class
    # Note that these should not be used directly if there is any chance of promotion.

    class ProductSet(PipelineProductSet):
        TelluricTransmission = IfuTelluric
        ResponseFunction = IfuReduced1d
        FluxcalTab = FluxCalTable

    class Qc(QcParameterSet):
        # QCs are apprently not very reusable, so we can define them here
        Chi2 = qc.ifu.IfuTelluricChi2
        NpThreshold = qc.ifu.IfuTelluricNpThreshold
        Conversion = qc.ifu.IfuTelluricConversion
    # TODO: Define input type for the paramfile in common.py

    # ++++++++++++++ Defining functions +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    # Invoke molecfit
    def mf_model(self):
        """
        Purpose: invoke molecfit to achieve a best-fit in the fitting regions
        """
        pass    # do nothing in the meantime

    # Invoke Calctrans
    def mf_calctrans(self):
        """
        Purpose: invoke calctrans to calculate transmission over the whole wavelength range
        """
        pass    # do nothing in the meantime

    # Recipe is at the moment also foreseen to create the response curve for the flux calibration
    # Response determination
    def determine_response(self):
        """
        Purpose: determine response function, i.e. compare observed standard star spectrum with the model in REF_STD_CAT
        """
        pass    # do nothing in the meantime

    # Function to process everything?
    def process(self) -> set[DataItem]:
        # self.correct_telluric()
        # self.apply_fluxcal()
        self.mf_model()
        self.mf_calctrans()
        self.determine_response()

        header_transmission = create_dummy_header()
        header_reduced_1d = create_dummy_header()
        header_fluxcal_tab = create_dummy_header()
        image = create_dummy_image()
        table = create_dummy_table()

        _combined = self.inputset.combined.load_data('DET1.DATA')

        primary_header = create_dummy_header()
        product_telluric_transmission = self.ProductSet.TelluricTransmission(
            primary_header,
            Hdu(header_transmission, table, name='TABLE'),
        )
        product_reduced_1d = self.ProductSet.ResponseFunction(
            create_dummy_header(),
            Hdu(header_reduced_1d, image, name='DET1.DATA'),
        )
        product_fluxcal_tab = self.ProductSet.FluxcalTab(
            create_dummy_header(),
            Hdu(header_fluxcal_tab, table, name='TABLE'),
        )

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.Chi2(None),
            self.Qc.Conversion(None),
            self.Qc.NpThreshold(None),
        ))

        return {product_telluric_transmission, product_reduced_1d, product_fluxcal_tab}


class MetisIfuTelluric(Recipe):
    _name: str = "metis_ifu_telluric"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Derive telluric absorption correction and optionally flux calibration"

    _algorithm = """Extract 1D spectrum of science object or standard star.
    Compute telluric correction.
    Compute conversion to physical units as function of wave-length."""
    _matched_keywords: frozenset[str] = frozenset({'DET.DIT', 'DET.NDIT', 'DRS.IFU'})

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

    Impl = MetisIfuTelluricImpl
