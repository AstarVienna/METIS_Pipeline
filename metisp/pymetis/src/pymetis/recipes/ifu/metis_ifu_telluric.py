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
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.dataitems.common import FluxCalTable, IfuTelluric
from pymetis.dataitems.ifu.ifu import IfuReduced1d, IfuCombined
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.inputs import FluxstdCatalogInput, LsfKernelInput, AtmProfileInput
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table


# The aim of this recipe is twofold:
#   (a) to determine the transmission function for telluric absorption correction
#   (b) determination of the response function for the flux calibration
#
# Note that there will be most probably a redesign / split into more recipes to follow the approach
# implemented already in other ESO pipelines

class MetisIfuTelluricImpl(MetisRecipeImpl):
    """Implementation class for metis_ifu_telluric"""

    # ++++++++++++++ Defining input +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Define molecfit main input class as one 1d spectrum, either Science or Standard spectrum
    class InputSet(PipelineInputSet):
        """Inputs for metis_ifu_telluric"""
        # TODO: still needs to be added to the input set
        # class Reduced1DInput(SinglePipelineInput):
        #     _tags: re.Pattern = re.compile(rf"IFU_(?P<target>SCI|STD)_1D")
        #     _group = cpl.ui.Frame.FrameGroup.CALIB
        #     _title: str = "uncorrected mf input spectrum"
        #     _description: str = "Uncorrected MF input spectrum."

        class CombinedInput(SinglePipelineInput):
            Item = IfuCombined

        FluxstdCatalogInput = FluxstdCatalogInput
        LsfKernelInput = LsfKernelInput
        AtmProfileInput = AtmProfileInput

    # ++++++++++++++ Defining ouput +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Recipe is foreseen to do both, create transmission and response functions
    # We therefore need to define transmission spectrum and response curve class
    # Note that these should not be used directly if there is any chance of promotion.

    ProductTelluricTransmission = IfuTelluric
    ProductResponseFunction = IfuReduced1d
    ProductFluxcalTab = FluxCalTable

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

        primary_header = create_dummy_header()
        header_transmission = create_dummy_header()
        header_reduced_1d = create_dummy_header()
        header_fluxcal_tab = create_dummy_header()
        image = create_dummy_image()
        table = create_dummy_table()

        combined = self.inputset.combined.load_data('DET1.DATA')

        product_telluric_transmission = self.ProductTelluricTransmission(
            primary_header,
            Hdu(header_transmission, table, name='TABLE'),
        )
        product_reduced_1d = self.ProductResponseFunction(
            primary_header,
            Hdu(header_reduced_1d, image, name='DET1.DATA'),
        )
        product_fluxcal_tab = self.ProductFluxcalTab(
            primary_header,
            Hdu(header_fluxcal_tab, table, name='TABLE'),
        )

        return {product_telluric_transmission, product_reduced_1d, product_fluxcal_tab}


class MetisIfuTelluric(MetisRecipe):
    _name: str = "metis_ifu_telluric"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Derive telluric absorption correction and optionally flux calibration"

    _algorithm = """Extract 1D spectrum of science object or standard star.
    Compute telluric correction.
    Compute conversion to physical units as function of wave-length."""
    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}

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
