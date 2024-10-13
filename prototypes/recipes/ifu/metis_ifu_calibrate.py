import cpl
from cpl.core import Msg
from typing import Dict

from prototypes.base.impl import MetisRecipe, MetisRecipeImpl
from prototypes.base.input import RecipeInput
from prototypes.base.product import PipelineProduct
from prototypes.inputs import SinglePipelineInput, PipelineInputSet


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
            for product in [self.ProductSciCubeCalibrated(header)]
        }
        return self.products


class MetisIfuCalibrate(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Calibrate IFU science data"
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([])
    implementation_class = MetisIfuCalibrateImpl
