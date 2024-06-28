from typing import Any, Dict

import sys
sys.path.append('.')

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl


class MetisIfuReduce(MetisRecipeImpl):
    # Fill in recipe information
    _name = "metis_ifu_reduce"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    # The recipe will have a single enumeration type parameter, which allows the
    # user to select the frame combination method.
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_reduce.telluric",
            context="metis_ifu_reduce",
            description="Apply telluric correction",
            default=False,
            alternatives=(True, False),
        ),
    ])

    def __init__(self):
        super().__init__()
        self.reduced = None
        self.background = None
        self.reduced_cube = None
        self.combined_cube = None

    def load_frameset(self, frameset: cpl.ui.FrameSet) -> cpl.ui.FrameSet:
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

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        super().run(frameset, settings)

        master_dark = None

        # TODO: Detect detector
        # TODO: Twilight
        output_file = "MASTER_IMG_FLAT_LAMP.fits"

        # For demonstration purposes we raise an exception here. Real world
        # recipes should rather print a message (also to have it in the log file)
        # and exit gracefully.
        if len(self.raw_frames) == 0:
            raise cpl.core.DataNotFoundError("No raw frames in frameset.")

        if master_dark is None:
            raise cpl.core.DataNotFoundError("No masterdark frames in frameset.")

        # By default images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.

        masterdark_image = cpl.core.Image.load(master_dark.file, extension=0)

        self.header = None
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            # Subtract dark
            raw_image.subtract(masterdark_image)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
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

    def add_product_properties(self) -> None:
        # Create property list specifying the product tag of the processed image
        product_properties = core.PropertyList()
        self.product_properties.append(
            # TODO: Other detectors
            # TODO: Twilight
            core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_IFU_REDUCE")
        )

        # Save the result image as a standard pipeline product file
        Msg.info(self.name, f"Saving product file as {self.output_file!r}.")
        dfs.save_image(
            self.frameset,
            self.parameters,
            self.frameset,
            self.combined_image,
            self.name,
            self.product_properties,
            f"demo/{self.version!r}",
            self.output_file_name,
            header=header,
        )

        # Register the created product
        product_frames.append(
            ui.Frame(
                file=output_file,
                tag="MASTER_IMG_FLAT_LAMP_2RG",
                group=ui.Frame.FrameGroup.PRODUCT,
                level=ui.Frame.FrameLevel.FINAL,
                frameType=ui.Frame.FrameType.IMAGE,
            )
        )

        return product_frames

    @property
    def detector_name(self) -> str:
        """ All IFU recipes work with the HAWAII2RG array """
        return "2RG"

    @property
    def output_file_name(self) -> str:
        return f"IFU_SCI_REDUCED"