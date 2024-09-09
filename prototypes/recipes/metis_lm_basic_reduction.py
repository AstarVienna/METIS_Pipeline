from typing import Any, Dict
from schema import Schema

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.product import PipelineProduct
from prototypes.rawimage import RawImageProcessor


class MetisLmBasicReductionImpl(RawImageProcessor):
    class Input(RawImageProcessor.Input):
        bias: cpl.ui.Frame
        flat: cpl.ui.Frame
        gain: cpl.ui.Frame

        def categorize_frame(self, frame):
            if frame.tag == "LM_IMAGE_SCI_RAW":
                frame.group = cpl.ui.Frame.FrameGroup.RAW
                self.raw.append(frame)
                Msg.debug(self.__class__.__name__, f"Got raw frame: {frame.file}.")
            elif frame.tag == "MASTER_DARK_2RG":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.master_dark = frame
                Msg.debug(self.__class__.__name__, f"Got bias frame: {frame.file}.")
            elif frame.tag == "MASTER_GAIN_2RG":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.master_gain = frame
                Msg.debug(self.__class__.__name__, f"Got bias frame: {frame.file}.")
            elif frame.tag == "MASTER_IMG_FLAT_LAMP_LM":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.master_flat = frame
                Msg.debug(self.__class__.__name__, f"Got bias frame: {frame.file}.")
            elif frame.tag == "MASTER_FLAT_LAMP":
                frame.group = cpl.ui.Frame.FrameGroup.CALIB
                self.master_flat = frame
                Msg.debug(self.__class__.__name__, f"Got flat field frame: {frame.file}.")
            else:
                super().categorize_frame(frame)

        def verify(self):
            super().verify()
            #import pdb; pdb.set_trace()
            
            try:
                self.bias = cpl.core.Image.load(self.master_dark.file, extension=0)
                Msg.info(self.__class__.__name__, f"Loaded bias frame {self.master_dark.file!r}.")
            except:
                raise cpl.core.DataNotFoundError("No bias frame in frameset.")
            
            try:
                self.gain = cpl.core.Image.load(self.master_gain.file, extension=0)
                Msg.info(self.__class__.__name__, f"Loaded bias frame {self.master_gain.file!r}.")
            except:
                raise cpl.core.DataNotFoundError("No bias frame in frameset.")

            try:
                self.flat = cpl.core.Image.load(self.master_flat.file, extension=0)
                Msg.info(self.__class__.__name__, f"Loaded flat frame {self.master_flat.file!r}.")
            except:
                raise cpl.core.DataNotFoundError("No flat frame in frameset.")

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
        

    def verify_input_frames(self) -> None:
        """ RawImageProcessor mixin wants to see a bunch of raw frames. """
        pass

    
    def process_images(self) -> Dict[str, PipelineProduct]:
        method = self.parameters["basic_reduction.stacking.method"].value
        Msg.info(self.__class__.__name__, f"Combining images using method {method!r}")
        #import pdb; pdb.set_trace()
        Msg.info(self.__class__.__name__, f"Starting processing image attibute.")
        raw_images = self.load_input_images()
        
        Msg.info(self.__class__.__name__, f"Detector name = {self.detector_name}")

        combined_image = cpl.core.ImageList()
        
        match method:
            case "add":
                for idx, image in enumerate(raw_images):
                    Msg.info(self.__class__.__name__, f"Processing frame #{idx}: {self.input.raw[idx].file}...")
                    combined_image.append(image)
            case "average":
                combined_image = raw_images.collapse_create()
            case "median":
                combined_image = raw_images.collapse_median_create()
            case _:
                Msg.error(self.__class__.__name__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")

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
