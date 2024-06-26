from typing import Any, Dict

import cpl
from cpl import dfs
from cpl.core import Msg

# TODO: fix importing. Where is the actual working directory?
import sys
sys.path.append('.')
from prototypes.base import MetisRecipe


class MetisDetDark(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_dark"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master dark"
    _description = (
        "Prototype to create a METIS Masterdark."
    )

    # The recipe will have a single enumeration type parameter, which allows the
    # user to select the frame combination method.
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_det_dark.stacking.method",
            context="metis_det_dark",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])

    def __init__(self):
        super().__init__()
        self.combined_image = None

    def load_input(self, frameset) -> cpl.ui.FrameSet:
        """ Go through the list of input frames, check the tag and act accordingly """

        for frame in frameset:
            # TODO: N and GEO
            match frame.tag:
                case "DARK_LM_RAW":                 # Should be DARK_IFU_RAW?
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw_frames.append(frame)
                    Msg.debug(self.name, f"Got raw frame: {frame.file}.")
                case _:
                    Msg.warning(
                        self.name,
                        f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.",
                    )

        # For demonstration purposes we raise an exception here. Real world
        # recipes should rather print a message (also to have it in the log file)
        # and exit gracefully.
        if len(self.raw_frames) == 0:
            raise cpl.core.DataNotFoundError("No raw frames in frameset.")

        return self.raw_frames

    def process_images(self) -> cpl.ui.FrameSet:
        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["metis_det_dark.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        processed_images = self.raw_images
        if method == "add":
            for idx, image in enumerate(processed_images):
                if idx == 0:
                    self.combined_image = image
                else:
                    self.combined_image.add(image)
        elif method == "average":
            self.combined_image = processed_images.collapse_create()
        elif method == "median":
            self.combined_image = processed_images.collapse_median_create()
        else:
            Msg.error(
                self.name,
                f"Got unknown stacking method {method!r}. Stopping right here!",
            )
            # Since we did not create a product we need to return an empty
            # ui.FrameSet object. The result frameset product_frames will do,
            # it is still empty here!
        return self.product_frames

        # Save the result image as a standard pipeline product file

    def add_product_properties(self) -> None:
        # Create property list specifying the product tag of the processed image
        self.product_properties.append(
            # TODO: Other detectors
            cpl.core.Property("ESO PRO CATG", cpl.core.Type.STRING, r"MASTER_DARK_2RG")
        )

    def save_product(self) -> cpl.ui.FrameSet:
        """ Register the created product """
        Msg.info(self.name, f"Saving product file as {self.output_file_name!r}.")
        dfs.save_image(
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

        self.product_frames = cpl.ui.FrameSet([
            cpl.ui.Frame(
                file=self.output_file_name,
                tag=rf"MASTER_DARK_{self.detector_name}",
                group=cpl.ui.Frame.FrameGroup.PRODUCT,
                level=cpl.ui.Frame.FrameLevel.FINAL,
                frameType=cpl.ui.Frame.FrameType.IMAGE,
            )
        ])

        return self.product_frames

    @property
    def detector_name(self) -> str:
        if self.header is None:
            return NotImplemented
        else:
            match self.header['ESO DPR TECH'].value:
                case "IMAGE,LM":
                    return "2RG"
                case "IMAGE,N":
                    return "GEO"
                case "IFU":
                    return "IFU"

    @property
    def output_file_name(self):
        """ Form the output file name (the detector part can change) """
        return f"MASTER_DARK_{self.detector_name}.fits"