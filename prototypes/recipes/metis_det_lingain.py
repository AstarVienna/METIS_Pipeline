from typing import Any, Dict

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipe
from prototypes.rawimage import RawImageProcessor
from prototypes.darkimage import DarkImageProcessor
from prototypes.product import PipelineProduct, DetectorProduct


class MetisDetLinGainImpl(DarkImageProcessor):
    class Input(DarkImageProcessor.Input):
        tags_raw: [str] = ["DETLIN_DET_RAW"]
        tags_dark: [str] = ["MASTER_DARK_2RG"]

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.bias_frame: cpl.ui.Frame | None = None
            super().__init__(frameset)

    class ProductGain(DetectorProduct):
        @property
        def category(self) -> str:
            return f"GAIN_MAP_{self.detector}"

    class ProductLinearity(DetectorProduct):
        @property
        def category(self) -> str:
            return f"LINEARITY_{self.detector}"

    class ProductBadpixMap(DetectorProduct):
        @property
        def category(self) -> str:
            return f"BADPIX_MAP_{self.detector}"

    def load_input_images(self) -> cpl.core.ImageList:
        pass

    def process_images(self) -> Dict[str, PipelineProduct]:
        raw_images = self.load_input_images()
        combined_image = self.combine_images(raw_images,
                                             method=self.parameters["metis_det_lingain.stacking.method"].value)

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)
        header = cpl.core.PropertyList.load(self.input.raw[0].file, 0)

        gain_image = combined_image # TODO fix
        linearity_image = combined_image # TODO fix
        badpix_map = combined_image # TODO fix

        self.products = {
            f'MASTER_GAIN_{self.detector_name}':
                self.ProductGain(self.recipe, header, gain_image, detector=self.detector_name),
            f'LINEARITY_{self.detector_name}':
                self.ProductLinearity(self.recipe, header, linearity_image, detector=self.detector_name),
            f'BADPIX_MAP_{self.detector_name}':
                self.ProductBadpixMap(self.recipe, header, badpix_map, detector=self.detector_name),
        }

        return self.products


class MetisDetLinearGain(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_lingain"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Measure detector non-linearity and gain"
    _description = (
        "Prototype to create a METIS linear gain map."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_det_lingain.stacking.method",
            context="metis_det_lingain",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
        cpl.ui.ParameterValue(
            name="metis_det_lingain.threshold.lowlim",
            context=_name,
            description="Thresholding threshold lower limit",
            default=0,
        ),
        cpl.ui.ParameterValue(
            name="metis_det_lingain.threshold.uplim",
            context=_name,
            description="Thresholding threshold upper limit",
            default=0,
        ),
    ])
    implementation_class = MetisDetLinGainImpl



