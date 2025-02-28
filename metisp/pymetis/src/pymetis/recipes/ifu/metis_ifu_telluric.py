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

import re

import cpl

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.inputs.common import FluxstdCatalogInput, LsfKernelInput, AtmProfileInput
from pymetis.products.product import PipelineProduct
from pymetis.products.common import TargetSpecificProduct


# The aim of this recipe is twofold:
#   (a) to determine the transmission function for telluric absorption correction
#   (b) determination of the response function for the flux calibration
#
# Note that there will be most probably a redesign / split into more recipes to follow the approach
# implemented already in other ESO pipelines

class MetisIfuTelluricImpl(MetisRecipeImpl):
    """Implementation class for metis_ifu_telluric"""

    detector = "IFU"

    # ++++++++++++++ Defining input +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Define molecfit main input class as one 1d spectrum, either Science or Standard spectrum
    class InputSet(PipelineInputSet):
        """Inputs for metis_ifu_telluric"""
        # TODO: still needs to be added to the input set
        #class Reduced1DInput(SinglePipelineInput):
        #    _tags: re.Pattern = re.compile(rf"IFU_(?P<target>SCI|STD)_1D")
        #    _group = cpl.ui.Frame.FrameGroup.CALIB
        #    _title: str = "uncorrected mf input spectrum"
        #    _description: str = "Uncorrected MF input spectrum."

        class CombinedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(rf"IFU_(?P<target>SCI|STD)_COMBINED")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "spectral cube of science object"
            _description: str = "Spectral cube of standard star, combining multiple exposures."

        FluxstdCatalogInput = FluxstdCatalogInput
        LsfKernelInput = LsfKernelInput
        AtmProfileInput = AtmProfileInput

    # ++++++++++++++ Defining ouput +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Recipe is foreseen to do both, create transmission and response functions
    # We therefore need to define transmission spectrum and response curve class

    # Tranmission spectrum
    class ProductTelluricTransmission(PipelineProduct):
        """
        Final product: Transmission function for the telluric correction
        """
        title = "Telluric correction"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Transmission function for the telluric correction"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return r"IFU_TELLURIC"

    # Response curve
    class ProductResponseFunction(TargetSpecificProduct):
        """
        Final product: response curve for the flux calibration
        """
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "response curve for the flux calibration"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return rf"IFU_{cls.target():s}_REDUCED_1D"

        @classmethod
        def description(cls) -> str:
            target = {
                'STD': 'reduced telluric standard star',
                'SCI': 'science object',
            }.get(cls.target(), '{target}')
            return f"Spectrum of a {target}."

    class ProductFluxcalTab(PipelineProduct):
        _tag = r"FLUXCAL_TAB"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE
        _description = "Conversion between instrumental and physical flux units."
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

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
    def process_images(self) -> [PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()
        self.mf_model()
        self.mf_calctrans()
        self.determine_response()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product_telluric_transmission = self.ProductTelluricTransmission(self, header, image)
        product_reduced_1d = self.ProductResponseFunction(self, header, image)
        product_fluxcal_tab = self.ProductFluxcalTab(self, header, image)

        return [product_telluric_transmission, product_reduced_1d, product_fluxcal_tab]

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        return {
            'STD': MetisIfuTelluricStdImpl,
            'SCI': MetisIfuTelluricSciImpl,
        }[self.inputset.target]


class MetisIfuTelluricStdImpl(MetisIfuTelluricImpl):
    class InputSet(MetisIfuTelluricImpl.InputSet):
        pass

    class ProductResponseFunction(MetisIfuTelluricImpl.ProductResponseFunction):
        _target = 'STD'


class MetisIfuTelluricSciImpl(MetisIfuTelluricImpl):
    class InputSet(MetisIfuTelluricImpl.InputSet):
        pass

    class ProductResponseFunction(MetisIfuTelluricImpl.ProductResponseFunction):
        _target = 'SCI'



class MetisIfuTelluric(MetisRecipe):
    _name: str = "metis_ifu_telluric"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Derive telluric absorption correction and optionally flux calibration"
    _description: str = """
        Recipe to derive the atmospheric transmission and the response function.

        Inputs
            IFU_(SCI|STD)_1D: 1d spectrum either from science target or standard star

        Outputs
            IFU_TELLURIC:   Tranmission of the Earth's atmosphere
            FLUXCAL_TAB:    Response function

        Algorithm
            *TBwritten*
    """

    _algorithm = """Extract 1D spectrum of science object or standard star.
        Compute telluric correction.
        Compute conversion to physical units as function of wave-length."""
    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}

    implementation_class = MetisIfuTelluricImpl
