from typing import Any, Dict

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl
from prototypes.product import PipelineProduct


class MetisLMBasicReductionImpl(MetisRecipeImpl):
    class Product(PipelineProduct):

        def as_frame(self) -> cpl.ui.Frame:
            return cpl.ui.Frame(file=self.output_file_name,
                                tag="OBJECT_REDUCED",
                                group=cpl.ui.Frame.FrameGroup.PRODUCT,
                                level=cpl.ui.Frame.FrameLevel.FINAL,
                                frameType=cpl.ui.Frame.FrameType.IMAGE)

        @property
        def category(self) -> str:
            return "OBJECT_REDUCED"

        @property
        def output_file_name(self):
            return f"{self.category}.fits"

    def __init__(self, recipe):
        super().__init__(recipe)
        self.bias_frame = None
        self.bias_image = None
        self.flat_frame = None
        self.flat_image = None
        self.raw_frames = cpl.ui.FrameSet()

    def categorize_frameset(self) -> cpl.ui.FrameSet:
        for frame in self.frameset:
            if frame.tag == "LM_IMAGE_SCI_RAW":
                frame.group = cpl.ui.Frame.FrameGroup.RAW
                self.raw_frames.append(frame)
                Msg.debug(self.name, f"Got raw frame: {frame.file}.")
            elif frame.tag == "MASTER_DARK_2RG":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.bias_frame = frame
                Msg.debug(self.name, f"Got bias frame: {frame.file}.")
            elif frame.tag == "MASTER_FLAT_LAMP":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.flat_frame = frame
                Msg.debug(self.name, f"Got flat field frame: {frame.file}.")
            else:
                Msg.warning(self.name, f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

    def verify_input(self) -> None:
        # For demonstration purposes we raise an exception here. Real world
        # recipes should rather print a message (also to have it in the log file)
        # and exit gracefully.
        if len(self.raw_frames) == 0:
            raise cpl.core.DataNotFoundError("No raw frames in frameset.")

        # By default images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.
        if self.bias_frame:
            self.bias_image = cpl.core.Image.load(self.bias_frame.file, extension=0)
            Msg.info(self.name, f"Loaded bias frame {self.bias_frame.file!r}.")
        else:
            #raise core.DataNotFoundError("No bias frame in frameset.")
            Msg.warning(self.name, "No bias frame in frameset.")

        flat_image = None
        if self.flat_frame:
            self.flat_image = cpl.core.Image.load(self.flat_frame.file, extension=0)
            Msg.info(self.name, f"Loaded flat frame {self.flat_frame.file!r}.")
        else:
            # raise core.DataNotFoundError("No flat frame in frameset.")
            Msg.warning(self.name, "No flat frame in frameset.")



class MetisLMBasicReduction(cpl.ui.PyRecipe):
    # Fill in recipe information
    _name = "metis_lm_basic_reduction"
    _version = "0.1"
    _author = "Chi-Hung Yan"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic science image data processing"
    _description = (
        "The recipe combines all science input files in the input set-of-frames using\n"
        + "the given method. For each input science image the master bias is subtracted,\n"
        + "and it is divided by the master flat."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="basic_reduction.stacking.method",
            context="basic_reduction",
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        )
    ])
    implementation_class = MetisLMBasicReductionImpl

    def __init__(self) -> None:
        super().__init__()
        self.implementation = self.implementation_class(self)

    def run(self, frameset: ui.FrameSet, settings: Dict[str, Any]) -> ui.FrameSet:


        raw_frames = ui.FrameSet()
        product_frames = ui.FrameSet()
        bias_frame = None
        flat_frame = None

        output_file = "OBJECT_REDUCED.fits"


        # Flat field preparation: subtract bias and normalize it to median 1
        Msg.info(self.name, "Preparing flat field")
        if self.flat_image:
            if self.bias_image:
                flat_image.subtract(bias_image)
            median = flat_image.get_median()
            flat_image.divide_scalar(median)

        header = None
        processed_images = core.ImageList()
        for idx, frame in enumerate(raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            if self.bias_image:
                Msg.debug(self.name, "Bias subtracting...")
                raw_image.subtract(self.bias_image)

            if self.flat_image:
                Msg.debug(self.name, "Flat fielding...")
                raw_image.divide(flat_image)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            processed_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["basic_reduction.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        combined_image = None
        if method == "add":
            for idx, image in enumerate(processed_images):
                if idx == 0:
                    combined_image = image
                else:
                    combined_image.add(image)
        elif method == "average":
            combined_image = processed_images.collapse_create()
        elif method == "median":
            combined_image = processed_images.collapse_median_create()
        else:
            Msg.error(
                self.name,
                f"Got unknown stacking method {method!r}. Stopping right here!",
            )
            # Since we did not create a product we need to return an empty
            # ui.FrameSet object. The result frameset product_frames will do,
            # it is still empty here!
            return product_frames

        return product_frames
