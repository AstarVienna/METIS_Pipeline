import cpl
from cpl.core import Msg
from typing import Any, Dict, Literal

from prototypes.base import MetisRecipe
from prototypes.darkimage import DarkImageProcessor
from prototypes.mixins import PersistenceInputMixin, BadpixMapInputMixin, LinearityInputMixin
from prototypes.mixins.detectors import Detector2rgMixin
from prototypes.product import PipelineProduct


class MetisIfuDistortionImpl(DarkImageProcessor):
    kind: Literal["SCI"] | Literal["STD"] = None

    class Input(Detector2rgMixin, PersistenceInputMixin, LinearityInputMixin, BadpixMapInputMixin,
                DarkImageProcessor.Input):
        tags_raw = ["IFU_DISTORTION_RAW"]
        tags_dark = ["MASTER_DARK_IFU"]
        tag_pinhole = "PINHOLE_TABLE"
        detector_name = '2RG'

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
            pass

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        self.products = {
            product.category: product
            for product in [self.ProductSciCubeCalibrated]
        }
        return self.products

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        super().run(frameset, settings)

        self.header = None
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.input_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            # Subtract dark
            raw_image.subtract(masterdark_image)

            # Insert the processed image in an image list.
            # Of course there is also an append() method available.
            raw_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["metis_ifu_calibrate.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        combined_image = None
        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        processed_images = raw_images
        match method:
            case "add":
                for idx, image in enumerate(processed_images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = processed_images.collapse_create()
            case "median":
                combined_image = processed_images.collapse_median_create()
            case _:
                Msg.error(
                    self.name,
                    f"Got unknown stacking method {method!r}. Stopping right here!",
                )
                # Since we did not create a product, we need to return an empty
                # ui.FrameSet object. The result frameset product_frames will do,
                # it is still empty here!
                return self.product_frames


    @property
    def detector_name(self) -> str:
        """ All IFU recipes work with the HAWAII2RG array """
        return "2RG"

    @property
    def output_file_name(self) -> str:
        return f"IFU_SCI_REDUCED"


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
