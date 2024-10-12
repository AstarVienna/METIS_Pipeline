import cpl
from cpl.core import Msg
from typing import Dict, Literal

from prototypes.base.impl import MetisRecipe
from prototypes.prefabricates.darkimage import DarkImageProcessor
from prototypes.mixins import PersistenceInputMixin, BadpixMapInputMixin, LinearityInputMixin, GainMapInputMixin
from prototypes.mixins.detectors import Detector2rgMixin
from prototypes.base.product import PipelineProduct


class MetisIfuDistortionImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class Input(Detector2rgMixin, PersistenceInputMixin, LinearityInputMixin, GainMapInputMixin, BadpixMapInputMixin,
                DarkImageProcessor.Input):
        tags_raw = ["IFU_DISTORTION_RAW"]
        tags_dark = ["MASTER_DARK_IFU"]
        tag_pinhole = "PINHOLE_TABLE"

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.pinhole_table: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case self.tag_pinhole:
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.pinhole_table = self._override_with_warning(self.pinhole_table, frame,
                                                                     origin=self.__class__.__qualname__,
                                                                     title="pinhole table")
                    Msg.debug(self.__class__.__qualname__, f"Got a pinhole table frame: {frame.file}.")
                case _:
                    super().categorize_frame(frame)

        def verify(self):
            """
            This Input is just a simple composition of mixins and does not require any further action
            """
            pass

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        masterdark_image = cpl.core.Image.load(self.input.master_dark.file)
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.input.raw_frames):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_image.subtract(masterdark_image)
            raw_images.append(raw_image)

        method = self.parameters["metis_ifu_calibrate.stacking.method"].value
        combined_image = self.combine_images(raw_images, method)

        self.products = {
            product.category: product(self, self.header, combined_image, detector_name=self.detector_name)
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products


class MetisIfuCalibrate(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([])
    implementation_class = MetisIfuDistortionImpl
