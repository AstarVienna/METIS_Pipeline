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
from typing import Dict

import cpl

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.inputs.common import FluxTableInput, LsfKernelInput, AtmProfileInput


# The aim of this recipe is twofold,
#   (a) to determine the transmission function for telluric absorption correction
#   (b) determination of the response function for the flux calibration
#
# Note that there will be most probably a redesign / split into more recipes to follow the approach
# implemented already in other ESO pipelines

class MetisIfuTelluricImpl(MetisRecipeImpl):
    """Implementation class for metis_ifu_telluric"""

    # Defining detector name
    @property
    def detector_name(self) -> str | None:
        return "IFU"

    # ++++++++++++++ Defining input +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Define molecfit main input class as one 1d spectrum, either Science or Standard spectrum
    class InputSet(PipelineInputSet):
        """Inputs for metis_ifu_telluric"""
        class Reduced1DInput(SinglePipelineInput):
            _tags = re.compile(rf"IFU_(?P<target>SCI|STD)_1D")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "uncorrected mf input spectrum"

        class CombinedInput(SinglePipelineInput):
            _tags = re.compile(rf"IFU_(?P<target>SCI|STD)_COMBINED")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "spectral cube of science object"

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.combined = self.CombinedInput(frameset)
            self.fluxstd_catalog = FluxTableInput(frameset)
            self.atm_profile = AtmProfileInput(frameset)
            self.lsf_kernel = LsfKernelInput(frameset)
            self.inputs |= {self.combined, self.fluxstd_catalog, self.atm_profile, self.lsf_kernel}

    # ++++++++++++++ Defining ouput +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Recipe is foreseen to do both, create transmission and response functions
    # We therefore need to define transmission spectrum and response curve class

    # Tranmission spectrum
    class ProductTelluricTransmission(PipelineProduct):
        """
        Final product: Transmission function for the telluric correction
        """
        level = cpl.ui.Frame.FrameLevel.FINAL
        tag = r"IFU_TELLURIC"
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    # Response curve
    class ProductResponseFunction(PipelineProduct):
        """
        Final product: response curve for the flux calibration
        """
        level = cpl.ui.Frame.FrameLevel.FINAL
        tag = r"FLUXCAL_TAB"
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    # TODO: Define input type for the paramfile in common.py

    # ++++++++++++++ Defining functions +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    # Invoke molecfit
    def mf_model(self):
        """
        Purpose: invoke molecfit to achieve a best-fit in the fitting regions
        """
        pass    # do nothing in the meanwhile

    # Invoke Calctrans
    def mf_calctrans(self):
        """
        Purpose: invoke calctrans to calculate transmission over the whole wavelength range
        """
        pass    # do nothing in the meanwhile

    # Recipe is in the moment also foreseen to create the response curve for the flux calibration
    # Response determination
    def determine_response(self):
        """
        Purpose: determine response function, i.e. compare observed standard star spectrum with the model in REF_STD_CAT
        """
        pass    # do nothing in the meanwhile

    # Function to process everything?
    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()
        self.mf_model()
        self.mf_calctrans()
        self.determine_response()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        self.products = {
            product.tag: product(self, header, image)
            for product in [self.ProductTelluricTransmission, self.ProductResponseFunction]
        }
        return self.products


class MetisIfuTelluric(MetisRecipe):
    _name = "metis_ifu_telluric"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Derive telluric absorption correction and optionally flux calibration"
    _description = """
        Recipe to derive the atmospheric transmission and the response function.

        Inputs
            IFU_SCI|STD_1D: 1d spectrum either from science target or standard star

        Outputs
            IFU_TELLURIC:   Tranmission of the Earth#s atmosphere
            FLUXCAL_TAB:    Response function

        Algorithm
            *TBwritten*
    """

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterValue(
            name=f"{_name}.dummy",
            context=_name,
            description="Dummy parameter",
            default="dummy",
        )
    ])
    implementation_class = MetisIfuTelluricImpl

