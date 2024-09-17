import cpl
from cpl.core import Msg
from typing import Any, Dict, Literal

from prototypes.base import MetisRecipe
from prototypes.darkimage import DarkImageProcessor
from prototypes.input import PipelineInput
from prototypes.product import PipelineProduct


class MetisIfuReduceImpl(DarkImageProcessor):
    kind: Literal["SCI"] | Literal["STD"] = None

    class Input(DarkImageProcessor.Input):
        tags_raw = ["IFU_SCI_RAW", "IFU_STD_RAW"]
        tags_dark = ["MASTER_DARK_IFU"]
        detector_name = '2RG'

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self._detector_name = "2RG"

        def verify(self):
            pass


    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def load_raw_images(self, frameset: cpl.ui.FrameSet) -> cpl.ui.FrameSet:
        for frame in frameset:
            match frame.tag:
                case "MASTER_DARK_IFU":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB

        return frameset

    def categorize_frame(self, frame: cpl.ui.Frame) -> cpl.ui.Frame:
        match frame.tag:
            case "MASTER_DARK_IFU":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
            case _:
                Msg.warning(self.name,
                            f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring it")

    def process_images(self) -> Dict[str, PipelineProduct]:
        self.products = {
            rf'IFU_{self.kind}_REDUCED': self.ProductReduced(),
            rf'IFU_{self.kind}_BACKGROUND': self.ProductBackground(),
            rf'IFU_{self.kind}_REDUCED_CUBE': self.ProductReducedCube(),
            rf'IFU_{self.kind}_COMBINED': self.ProductCombined(),
        }
        return self.products

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        super().run(frameset, settings)

        # TODO: Detect detector
        # TODO: Twilight
        output_file = "MASTER_IMG_FLAT_LAMP.fits"

        # For demonstration purposes we raise an exception here. Real world
        # recipes should rather print a message (also to have it in the log file)
        # and exit gracefully.

        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.

        masterdark_image = cpl.core.Image.load(master_dark.file, extension=0)

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
        method = self.parameters["metis_lm_img_flat.stacking.method"].value
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
                # Since we did not create a product we need to return an empty
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


class MetisIfuReduce(MetisRecipe):
    _name = "metis_ifu_reduce"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_reduce.telluric",
            context="metis_ifu_reduce",
            description="IFU basic data reduction",
            default=False,
            alternatives=(True, False),
        ),
    ])
    implementation_class = MetisIfuReduceImpl
