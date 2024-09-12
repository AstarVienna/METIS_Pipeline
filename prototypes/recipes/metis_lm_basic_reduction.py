from platform import processor
from typing import Any, Dict

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.product import PipelineProduct
from prototypes.rawimage import RawImageProcessor


class MetisLmBasicReductionImpl(RawImageProcessor):
    class Input(RawImageProcessor.Input):
        def __init__(self, frameset: cpl.ui.FrameSet):
            self.master_dark: cpl.ui.Frame | None = None
            self.master_flat: cpl.ui.Frame | None = None
            self.master_gain: cpl.ui.Frame | None = None
            self.bias: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame):
            match frame.tag:
                case "LM_IMAGE_SCI_RAW":
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw.append(frame)
                    Msg.debug(self.__class__.__qualname__, f"Got raw frame: {frame.file}.")
                case "MASTER_DARK_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_dark = frame
                    Msg.debug(self.__class__.__qualname__, f"Got master dark frame: {frame.file}.")
                case "MASTER_GAIN_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_gain = frame
                    Msg.debug(self.__class__.__qualname__, f"Got bias frame: {frame.file}.")
                case "MASTER_IMG_FLAT_LAMP_LM":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_flat = frame
                    Msg.debug(self.__class__.__qualname__, f"Got flat lamp frame: {frame.file}.")
                case "MASTER_FLAT_LAMP":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_flat = frame
                    Msg.debug(self.__class__.__qualname__, f"Got flat field frame: {frame.file}.")
                case _:
                    super().categorize_frame(frame)

        def verify(self):
            super().verify()

            if self.master_flat is None:
                raise cpl.core.DataNotFoundError("No master flat frames found in the frameset.")

            if self.master_gain is None:
                raise cpl.core.DataNotFoundError("No master gain frames found in the frameset.")

            if self.bias is None:
                raise cpl.core.DataNotFoundError("No bias frame found in the frameset.")

    class Product(PipelineProduct):
        tag: str = "OBJECT_REDUCED"
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return self.tag

        @property
        def output_file_name(self):
            return f"{self.category}.fits"

    def prepare_flat(self, flat: cpl.core.Image, bias: cpl.core.Image | None):
        """ Flat field preparation: subtract bias and normalize it to median 1 """
        Msg.info(self.__class__.__qualname__, "Preparing flat field")
        if flat is None:
            raise RuntimeError("No flat frames found in the frameset.")
        else:
            if bias is not None:
                flat.subtract(bias)
            median = flat.get_median()
            return flat.divide_scalar(median)

    def prepare_images(self,
                       raw_frames: cpl.ui.FrameSet, *,
                       bias: cpl.core.Image | None = None,
                       flat: cpl.core.Image | None = None) -> cpl.core.ImageList:
        prepared_images = cpl.core.ImageList()

        for index, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file!r}...")

            Msg.debug(self.__class__.__qualname__, "Loading image {}")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            if bias:
                Msg.debug(self.__class__.__qualname__, "Bias subtracting...")
                raw_image.subtract(bias)

            if flat:
                Msg.debug(self.__class__.__qualname__, "Flat fielding...")
                raw_image.divide(flat)

            prepared_images.append(raw_image)

        return prepared_images


    def process_images(self) -> Dict[str, PipelineProduct]:
        Msg.info(self.__class__.__qualname__, f"Starting processing image attibute.")

        dark = cpl.core.Image.load(self.input.master_dark.file, extension=0)
        flat = cpl.core.Image.load(self.input.master_flat.file, extension=0)
        bias = cpl.core.Image.load(self.input.master_bias.file, extension=0)
        gain = cpl.core.Image.load(self.input.master_gain.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector_name}")

        flat = self.prepare_flat(flat, bias)
        images = self.prepare_images(self.input.raw, flat, bias)
        combined_image = self.combine_images(images, self.parameters["basic_reduction.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.input.raw[0].file, 0)

        self.products = {
            fr'OBJECT_REDUCED_{self.detector_name}':
                self.Product(self, header, combined_image,
                             detector_name=self.detector_name),
        }

        return self.products

    @property
    def detector_name(self) -> str:
        return "2RG"


class MetisLmBasicReduction(MetisRecipe):
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
    implementation_class = MetisLmBasicReductionImpl

    def __init__(self) -> None:
        super().__init__()
        self.implementation = self.implementation_class(self)

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        raw_frames = cpl.ui.FrameSet()
        product_frames = cpl.ui.FrameSet()
        bias_frame = None
        flat_frame = None

        output_file = "OBJECT_REDUCED.fits"


        # Flat field preparation: subtract bias and normalize it to median 1
        Msg.info(self.__class__.__qualname__, "Preparing flat field")
        if self.flat_image:
            if self.bias_image:
                self.flat_image.subtract(self.bias_image)
            median = self.flat_image.get_median()
            self.flat_image.divide_scalar(median)

        header = None
        processed_images = cpl.core.ImageList()
        for idx, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file!r}...")

            if idx == 0:
                header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.__class__.__qualname__, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            if self.bias_image:
                Msg.debug(self.__class__.__qualname__, "Bias subtracting...")
                raw_image.subtract(self.bias_image)

            if self.flat_image:
                Msg.debug(self.__class__.__qualname__, "Flat fielding...")
                raw_image.divide(self.flat_image)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            processed_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["basic_reduction.stacking.method"].value
        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")

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
                self.__class__.__qualname__,
                f"Got unknown stacking method {method!r}. Stopping right here!",
            )
            # Since we did not create a product, we need to return an empty
            # `ui.FrameSet` object. The result frameset product_frames will do,
            # it is still empty here!
            return product_frames

        return product_frames
