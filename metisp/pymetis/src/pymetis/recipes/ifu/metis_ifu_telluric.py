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
from pymetis.base.input import RecipeInput
from pymetis.base.product import PipelineProduct


class MetisIfuTelluricImpl(MetisRecipeImpl):
    @property
    def detector_name(self) -> str | None:
        return "2RG"

    class ProductSciReduced1D(PipelineProduct):
        category = rf"IFU_SCI_REDUCED_1D"

    class ProductIfuTelluric(PipelineProduct):
        category = "IFU_TELLURIC"

    class ProductFluxcalTab(PipelineProduct):
        category = "FLUXCAL_TAB"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        self.products = {
            product.category: product()
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

    parameters = cpl.ui.ParameterList([])
    implementation_class = MetisIfuTelluricImpl
