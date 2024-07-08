from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl
from prototypes.product import PipelineProduct


class MetisLmImgFlatImpl(MetisRecipeImpl):
    class Product(PipelineProduct):
        def __init__(self, recipe, header, frame, **kwargs):
            super().__init__(recipe, header, frame, **kwargs)

        def create_frame(self):
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
            return "MASTER_IMG_FLAT_LAMP_LM.fits"

    def __init__(self, recipe) -> None:
        super().__init__(recipe)
        self.masterdark = None
        self.masterdark_image = None

    def load_input_frameset(self, frameset: cpl.ui.FrameSet) -> None:
        """ Go through the list of input frames, check the tags and act on it accordingly """
        for frame in frameset:
            match frame.tag:
                case "LM_FLAT_LAMP_RAW":
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw_frames.append(frame)
                    Msg.debug(self.name, f"Got raw frame: {frame.file}.")
                case tag if tag in ["MASTER_DARK_2RG", "MASTER_DARK_GEO", "MASTER_DARK_IFU"]:
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.masterdark = frame
                    Msg.debug(self.name, f"Got master dark frame: {frame.file}.")
                case _:
                    Msg.warning(self.name, f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

    def verify_input(self) -> None:
        if len(self.raw_frames) == 0:
            raise cpl.core.DataNotFoundError("No raw frames found in the frameset.")

        if self.masterdark is None:
            raise cpl.core.DataNotFoundError("No masterdark frames found in the frameset.")

    def process_images(self) -> cpl.ui.FrameSet:
        # TODO: Detect detector
        # TODO: Twilight
        output_file = "MASTER_IMG_FLAT_LAMP_LM.fits"

        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.

        self.masterdark_image = cpl.core.Image.load(self.masterdark.file, extension=0)

        # Subtract the dark from every raw image
        for raw_image in self.raw_images:
            raw_image.subtract(self.masterdark_image)

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters["metis_lm_img_flat.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not) should come here

        self.processed_images = self.raw_images
        combined_image = None

        match method:
            case "add":
                for idx, image in enumerate(self.processed_images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = self.processed_images.collapse_create()
            case "median":
                combined_image = self.processed_images.collapse_median_create()
            case _:
                Msg.error(
                    self.name,
                    f"Got unknown stacking method {method!r}. Stopping right here!",
                )
                # Since we did not create a product we need to return an empty
                # ui.FrameSet object. The result frameset product_frames will do,
                # it is still empty here!

        header = cpl.core.PropertyList.load(self.raw_frames[0].file, 0)

        self.products = {
            f'METIS_LM_IMG_FLAT':
                self.Product(self,
                             header, combined_image,
                             file_name=f"MASTER_IMG_FLAT_LAMP_LM.fits"),
        }
        return self.product_frames


    @property
    def detector_name(self) -> str:
        return "2RG"


class MetisLmImgFlat(cpl.ui.PyRecipe):
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

    def __init__(self):
        super().__init__()
        self.implementation = self.implementation_class(self)

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        return self.implementation.run(frameset, settings)
