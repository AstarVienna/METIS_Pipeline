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
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.mixins import PersistenceInputSetMixin


class ProductTelluric(PipelineProduct):
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE


class MetisIfuTelluricImpl(MetisRecipeImpl):
    detector = '2RG'

    class InputSet(PersistenceInputSetMixin):
        class CombinedInput(SinglePipelineInput):
            _title = "combined science and standard frames"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags = re.compile(r"IFU_(?P<target>SCI|STD)_COMBINED")

        class LsfKernelInput(SinglePipelineInput):
            _title = "LSF kernel"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags = re.compile(r"LSF_KERNEL")

        class AtmProfileInput(SinglePipelineInput):
            _title = "atmospheric profile"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags = re.compile(r"ATM_PROFILE")

        class FluxStdCatalogInput(SinglePipelineInput):
            _title = "flux std catalog"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags = re.compile(r"FLUXSTD_CATALOG")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.combined = self.CombinedInput(frameset)
            self.lsf_kernel = self.LsfKernelInput(frameset)
            self.atmospheric_profile = self.AtmProfileInput(frameset)
            self.fluxstd_catalog = self.FluxStdCatalogInput(frameset)

            self.inputs |= {self.combined, self.lsf_kernel, self.atmospheric_profile, self.fluxstd_catalog}


    class ProductSciReduced1D(ProductTelluric):
        tag = rf"IFU_SCI_REDUCED_1D"

    class ProductIfuTelluric(ProductTelluric):
        tag = "IFU_TELLURIC"

    class ProductFluxcalTab(ProductTelluric):
        tag = "FLUXCAL_TAB"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = cpl.core.PropertyList()
        image = cpl.core.Image.load(self.inputset.combined.frame.file)

        self.products = {
            str(product.category): product(self, header, image)
            for product in [self.ProductSciReduced1D, self.ProductIfuTelluric, self.ProductFluxcalTab]
        }
        return self.products


class MetisIfuTelluric(MetisRecipe):
    _name = "metis_ifu_telluric"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Derive telluric absorption correction and optionally flux calibration"
    _description = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuTelluricImpl

    # Dummy parameter to circumvent a potential bug in `pyesorex`
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterValue(
            name=f"{_name}.dummy",
            context=_name,
            description="Dummy parameter",
            default="dummy",
        )
    ])
