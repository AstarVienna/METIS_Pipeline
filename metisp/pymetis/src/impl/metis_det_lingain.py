from abc import ABC
from typing import Dict

import cpl


from inputs.common import RawInput
from prefabricates.darkimage import DarkImageProcessor
from base.product import PipelineProduct, DetectorProduct
from prefabricates.rawimage import RawImageProcessor


class LinGainProduct(DetectorProduct, ABC):
    group = cpl.ui.Frame.FrameGroup.PRODUCT
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE

    @property
    def tag(self) -> str:
        return rf"{self.category}"


class MetisDetLinGainImpl(DarkImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        detector = '2RG'

        class RawInput(RawInput):
            _tags = ["DETLIN_DET_RAW"]

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.raw = self.RawInput(frameset)

    class ProductGain(LinGainProduct):
        @property
        def category(self) -> str:
            return f"GAIN_MAP_{self.detector}"

    class ProductLinearity(LinGainProduct):
        @property
        def category(self) -> str:
            return f"LINEARITY_{self.detector}"

    class ProductBadpixMap(LinGainProduct):
        @property
        def category(self) -> str:
            return f"BADPIX_MAP_{self.detector}"

    def process_images(self) -> Dict[str, PipelineProduct]:
        raw_images = self.load_raw_images()
        combined_image = self.combine_images(raw_images,
                                             method=self.parameters["metis_det_lingain.stacking.method"].value)

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        gain_image = combined_image         # TODO Actual implementation missing
        linearity_image = combined_image    # TODO Actual implementation missing
        badpix_map = combined_image         # TODO Actual implementation missing

        #import pdb ; pdb.set_trace()
        self.products = {
            f'MASTER_GAIN_{self.detector_name}':
                self.ProductGain(self, header, gain_image,
                                 detector=self.detector_name),
            f'LINEARITY_{self.detector_name}':
                self.ProductLinearity(self, header, linearity_image,
                                      detector=self.detector_name),
            f'BADPIX_MAP_{self.detector_name}':
                self.ProductBadpixMap(self, header, badpix_map,
                                      detector=self.detector_name),
        }

        return self.products