import cpl
from typing import Dict

from impl.base import MetisRecipe, MetisRecipeImpl
from metisp.pymetis.src.base.product import PipelineProduct
from metisp.pymetis.src.inputs import SinglePipelineInput, PipelineInputSet


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        detector = '2RG'

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.sci_reduced: SinglePipelineInput(frameset, tags=["IFU_SCI_REDUCED"])
            self.telluric: SinglePipelineInput(frameset, tags=["IFU_TELLURIC"])
            self.fluxcal: SinglePipelineInput(frameset, tags=["FLUXCAL_TAB"])

            self.inputs += [self.sci_reduced, self.telluric, self.fluxcal]

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        self.products = {
            product.category: product()
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products
