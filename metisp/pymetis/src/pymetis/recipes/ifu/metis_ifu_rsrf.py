import cpl
from typing import Dict

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet, MultiplePipelineInput, BadpixMapInput, \
                           MasterDarkInput, LinearityInput, GainMapInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.mixins import Detector2rgMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class RawInput(MultiplePipelineInput):
    _tags = ["IFU_RSRF_RAW"]


class RsrfMasterDarkInput(Detector2rgMixin, MasterDarkInput):
    pass


class DistortionTableInput(SinglePipelineInput):
    _tags = ["IFU_DISTORTION_TABLE"]


class WavecalInput(SinglePipelineInput):
    _tags = ["IFU_WAVECAL"]


class MetisIfuRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector = '2RG'

        RawInput = RawInput
        MasterDarkInput = RsrfMasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.badpix_map = BadpixMapInput(frameset, required=False)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)
            self.wavecal = WavecalInput(frameset)

            self.inputs += [self.badpix_map, self.linearity, self.gain_map]

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


class MetisIfuRsrf(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Determine the relative spectral response function"
    _description = (
        "Currently just a skeleton prototype."
    )

    implementation_class = MetisIfuRsrfImpl
