import cpl
from typing import Dict

from impl.base import MetisRecipe, MetisRecipeImpl
from metisp.pymetis.src.base.product import PipelineProduct


class MetisIfuTelluricImpl(MetisRecipeImpl):
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
