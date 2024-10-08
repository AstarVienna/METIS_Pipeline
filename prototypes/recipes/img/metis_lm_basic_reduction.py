from typing import Any, Dict

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipe
from prototypes.product import PipelineProduct
from prototypes.darkimage import DarkImageProcessor

from prototypes.mixins import BadpixMapInputMixin, LinearityInputMixin, GainMapInputMixin
from prototypes.mixins.detectors import Detector2rgMixin

class MetisLmBasicReductionImpl(DarkImageProcessor):
    class Input(Detector2rgMixin, BadpixMapInputMixin, GainMapInputMixin, LinearityInputMixin, DarkImageProcessor.Input):
        tags_raw: [str] = [r"LM_IMAGE_SCI_RAW"]
        tags_dark: [str] = [r"MASTER_DARK_2RG"]
        tags_flat: [str] = [r"MASTER_FLAT_LAMP"]
        tags_gain: [str] = [r"MASTER_GAIN_2RG"]

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.master_flat: cpl.ui.Frame | None = None
            self.master_gain: cpl.ui.Frame | None = None
            self.linearity: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame):
            match frame.tag:
                case "GAIN_MAP_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_gain = frame
                    Msg.debug(self.__class__.__qualname__, f"Got master gain frame: {frame.file}.")
                case "BADPIX_MAP_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_badpix = frame
                    Msg.debug(self.__class__.__qualname__, f"Got master gain frame: {frame.file}.")
                case "MASTER_GAIN_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_gain = frame
                    Msg.debug(self.__class__.__qualname__, f"Got master gain frame: {frame.file}.")
                case "MASTER_IMG_FLAT_LAMP_LM":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_flat = frame
                    Msg.debug(self.__class__.__qualname__, f"Got flat lamp frame: {frame.file}.")
                case "MASTER_FLAT_LAMP":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.master_flat = frame
                    Msg.debug(self.__class__.__qualname__, f"Got flat field frame: {frame.file}.")
                case "LINEARITY_2RG":
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.linearity = frame
                    Msg.debug(self.__class__.__qualname__, f"Got linearity frame: {frame.file}.")
                case _:
                    super().categorize_frame(frame)

        def verify(self):
            super().verify()

            if self.master_flat is None:
                raise cpl.core.DataNotFoundError("No master flat frame found in the frameset.")

            if self.master_gain is None:
                raise cpl.core.DataNotFoundError("No master gain frame found in the frameset.")

            if self.linearity is None:
                Msg.warning(self.__class__.__qualname__,
                            "No linearity frame found, not correcting for linearity")

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
        
        #import pdb ; pdb.set_trace()
        if flat is None:
            raise RuntimeError("No flat frames found in the frameset.")
        else:
            if bias is not None:
                flat.subtract(bias)
            median = flat.get_median()
            return flat.divide_scalar(median)

    def prepare_images(self,
                       raw_frames: cpl.ui.FrameSet,
                       bias: cpl.core.Image | None = None,
                       flat: cpl.core.Image | None = None) -> cpl.core.ImageList:
        prepared_images = cpl.core.ImageList()

        for index, frame in enumerate(raw_frames):
            Msg.info(self.__class__.__qualname__, f"Processing {frame.file!r}...")

            Msg.debug(self.__class__.__qualname__, f"Loading image {frame.file!r}")
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

        flat = cpl.core.Image.load(self.input.master_flat.file, extension=0)
        bias = cpl.core.Image.load(self.input.master_dark.file, extension=0)
        gain = cpl.core.Image.load(self.input.master_gain.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector_name}")


        flat = self.prepare_flat(flat, bias)
        images = self.prepare_images(self.input.raw, flat, bias)
        combined_image = self.combine_images(images, self.parameters["basic_reduction.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.input.raw[0].file, 0)

        self.products = {
            fr'OBJECT_REDUCED_{self.detector_name}':
                self.Product(self, header, combined_image, detector_name=self.detector_name),
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
