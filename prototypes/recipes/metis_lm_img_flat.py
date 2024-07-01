import sys
sys.path.append('.')

from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl


class MetisLmImgFlatImpl(MetisRecipeImpl):
    def __init__(self, recipe) -> None:
        super().__init__(recipe)
        self.masterdark = None
        self.masterdark_image = None
        self.combined_image = None

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
            raise cpl.core.DataNotFoundError("No raw frames in frameset.")

        if self.masterdark is None:
            raise cpl.core.DataNotFoundError("No masterdark frames in frameset.")

    def process_images(self) -> cpl.ui.FrameSet:
        # TODO: Detect detector
        # TODO: Twilight
        output_file = "MASTER_IMG_FLAT_LAMP_LM.fits"
        print(frameset)

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
        if method == "add":
            for idx, image in enumerate(self.processed_images):
                if idx == 0:
                    self.combined_image = image
                else:
                    self.combined_image.add(image)
        elif method == "average":
            self.combined_image = self.processed_images.collapse_create()
        elif method == "median":
            self.combined_image = self.processed_images.collapse_median_create()
        else:
            Msg.error(
                self.name,
                f"Got unknown stacking method {method!r}. Stopping right here!",
            )
            # Since we did not create a product we need to return an empty
            # ui.FrameSet object. The result frameset product_frames will do,
            # it is still empty here!
            return self.product_frames

    def add_product_properties(self) -> None:
        # Create property list specifying the product tag of the processed image
        self.product_properties.append(
            # TODO: Twilight
<<<<<<< HEAD
            cpl.core.Property("ESO PRO CATG",
                              cpl.core.Type.STRING,
                              rf"MASTER_IMG_FLAT_LAMP_{self.detector_name}")
=======
                core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_IMG_FLAT_LAMP_LM")
>>>>>>> 0244970 (Correct product name again)
        )

    def save_product(self) -> cpl.ui.FrameSet:
        # Save the result image as a standard pipeline product file
        Msg.info(self.name, f"Saving product file as {self.output_file_name!r}.")
        cpl.dfs.save_image(
            self.frameset,
            self.parameters,
            self.frameset,
            self.combined_image,
            self.name,
            self.product_properties,
            f"demo/{self.version!r}",
            self.output_file_name,
            header=self.header,
        )

        # Register the created product
        self.product_frames.append(
            cpl.ui.Frame(
                file=self.output_file_name,
                tag=f"MASTER_IMG_FLAT_LAMP_{self.detector_name}",
                group=cpl.ui.Frame.FrameGroup.PRODUCT,
                level=cpl.ui.Frame.FrameLevel.FINAL,
                frameType=cpl.ui.Frame.FrameType.IMAGE,
            )
        )

        return self.product_frames

    @property
    def detector_name(self) -> str:
        return "2RG"

    @property
    def output_file_name(self) -> str:
        """ Form the output file name (currently a constant) """
        return "MASTER_IMG_FLAT_LAMP.fits"


class MetisLmImgFlat(cpl.ui.PyRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Gilles Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master flat"
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
