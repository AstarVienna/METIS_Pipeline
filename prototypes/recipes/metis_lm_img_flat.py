from typing import Dict, Any
from schema import Schema

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipe
from prototypes.product import PipelineProduct
from prototypes.rawimage import RawImageProcessor


class MetisLmImgFlatImpl(RawImageProcessor):
    input_schema = Schema({
        'raw': cpl.ui.FrameSet,
        'dark': cpl.ui.Frame,
    })

    class Input(RawImageProcessor.Input):
        raw: cpl.ui.FrameSet
        master_dark: cpl.ui.Frame

        def categorize_frame(self, frame):
            super().categorize_frame(frame)

            match frame.tag:
                case "LM_FLAT_LAMP_RAW":
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw.append(frame)
                    Msg.debug(self.__class__.__name__, f"Got raw frame: {frame.file}.")
                case tag if tag in ["MASTER_DARK_2RG", "MASTER_DARK_GEO", "MASTER_DARK_IFU"]:
                    self.master_dark = frame
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    Msg.debug(self.__class__.__name__, f"Got master dark frame: {frame.file}.")
                case _:
                    Msg.warning(self.__class__.__name__, f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

        def verify(self) -> None:
            # First verify the raw frames (provided by base class)
            super().verify()

            if self.master_dark is None:
                raise cpl.core.DataNotFoundError("No masterdark frames found in the frameset.")

    class Product(PipelineProduct):
        def as_frame(self):
            return cpl.ui.Frame(file=self.output_file_name,
                                tag=rf"MASTER_IMG_FLAT_LAMP_LM",
                                group=cpl.ui.Frame.FrameGroup.PRODUCT,
                                level=cpl.ui.Frame.FrameLevel.FINAL,
                                frameType=cpl.ui.Frame.FrameType.IMAGE)

        @property
        def category(self) -> str:
            return "MASTER_IMG_FLAT_LAMP_LM"

        @property
        def output_file_name(self) -> str:
            """ Form the output file name (currently a constant) """
            return f"{self.category}.fits"

    # Subtract the dark from every raw image
    def process_images(self) -> Dict[str, PipelineProduct]:
        # TODO: Detect detector
        # TODO: Twilight

        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.

        raw_images = self.load_input_images()
        master_dark = cpl.core.Image.load(self.input.master_dark.file, extension=0)

        for raw_image in raw_images:
            Msg.debug(__name__, f"Subtracting image {raw_image}")
            raw_image.subtract(master_dark)

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters["metis_lm_img_flat.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not) should come here

        raw_images = self.load_input_images()
        combined_image = None

        match method:
            case "add":
                for idx, image in enumerate(raw_images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = raw_images.collapse_create()
            case "median":
                combined_image = raw_images.collapse_median_create()
            case _:
                Msg.error(
                    self.name,
                    f"Got unknown stacking method {method!r}. Stopping right here!",
                )
                # Since we did not create a product we need to return an empty
                # ui.FrameSet object. The result frameset product_frames will do,
                # it is still empty here!

        header = cpl.core.PropertyList.load(self.input.raw[0].file, 0)

        self.products = {
            r'METIS_LM_IMG_FLAT':
                self.Product(self,
                             header, combined_image,
                             file_name=f"MASTER_IMG_FLAT_LAMP_LM.fits"),
        }
        return self.products

    @property
    def detector_name(self) -> str:
        return "2RG"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Gilles Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master flat for L/M band detectors"
    _description = (
        "Prototype to create a METIS Masterflat."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_flat.stacking.method",
            context="metis_lm_img_flat",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])
    implementation_class = MetisLmImgFlatImpl
