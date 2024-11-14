from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput
from pymetis.inputs.common import MasterDarkInput, LinearityInput, PersistenceMapInput, GainMapInput, MasterFlatInput
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisLmImgBasicReduceImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        """
        The first step of writing a recipe is to define an InputSet: the singleton class
        that wraps all the recipe inputs. It encapsulates the entire input and

        - defines which tags to look for, optionally with placeholders like `{det}`, that can be set globally
        - which of the inputs are optional (just set `required = False`)
        - provides mechanism for verification that the required frames are actually present

        Inputs are twofold:

        - children of SinglePipelineInput, which expect exactly one frame to be present (or at most one if optional).
            They will warn is multiple frames are found and keep the last one.
        - children of MultiplePipelineInput, which expect multiple frames with the same tag (usually RAWs).
            Again, the frame set may be empty if `required` is set to False.

        You may instantiate your inputs directly as SinglePipelineInput or MultiplePipelineInput with appropriate tags,
        or use or extend one of the predefined classes (see `pymetis.inputs.common`).

        The input is automatically verified (see the base InputSet class and its Single and Multiple children)
        and an exception is raised whenever something is amiss.
        """

        # This InputSet class derives from DarkImageProcessor.InputSet, which in turn inherits from
        # RawImageProcessor.InputSet. It already knows that it wants a RawInput and MasterDarkInput class,
        # but does not know about the tags yet. So here we define tags for the raw input
        class Raw(RawInput):
            _tags = ["LM_IMAGE_SCI_RAW", "LM_IMAGE_STD_RAW"]

        # Also one master flat is required. We use a prefabricated class
        class MasterFlat(MasterFlatInput):
            _tags = ["MASTER_IMG_FLAT_LAMP_LM", "MASTER_IMG_FLAT_TWILIGHT_LM"]

        # We could define the master dark explicitly too, but we can use a prefabricated class instead.
        # That already has its tags defined (for master darks it's always "MASTER_DARK_{det}"), so we just define
        # the detector and band. Those are now available for all Input classes here.
        # Of course, we could be more explicit and define them directly.

        detector: str = '2RG'
        band: str = 'LM'

        RawInput = Raw
        MasterDarkInput = MasterDarkInput

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.master_flat = self.MasterFlat(frameset,
                                               tags=["MASTER_IMG_FLAT_LAMP_{band}", "MASTER_IMG_FLAT_TWILIGHT_{band}"],
                                               band="LM", det=self.detector)
            self.linearity = LinearityInput(frameset, det=self.detector)
            self.persistence = PersistenceMapInput(frameset, required=False)
            self.gain_map = GainMapInput(frameset, det=self.detector)

            # We need to register the inputs (just to be able to do `for x in self.inputs:`)
            self.inputs += [self.master_flat, self.linearity, self.persistence, self.gain_map]

    class Product(PipelineProduct):
        """
        The second big part is defining the products. For every product we create a separate class
        which defines the tag, group, level and frame type.
        """
        tag: str = "LM_{target}_REDUCED"
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
            return flat

            # return flat.divide_scalar(median)

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
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions, or use functions from the parent classes:
        for instance combine_images is a helper function that takes a frameset and a method and returns
        a single combined frame that is used throughout the pipeline.
        """

        Msg.info(self.__class__.__qualname__, f"Starting processing image attribute.")

        flat = cpl.core.Image.load(self.inputset.master_flat.frame.file, extension=0)
        bias = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)
        gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)

        Msg.info(self.__class__.__qualname__, f"Detector name = {self.detector_name}")

        flat = self.prepare_flat(flat, bias)
        images = self.prepare_images(self.inputset.raw.frameset, flat, bias)
        combined_image = self.combine_images(images, self.parameters["basic_reduction.stacking.method"].value)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        self.products = {
            fr'OBJECT_REDUCED_{self.detector_name}':
                self.Product(self, header, combined_image, detector_name=self.detector_name),
        }

        return self.products


class MetisLmImgBasicReduce(MetisRecipe):
    """
    Apart from our own recipe implementation we have to provide the actual recipe for PyEsoRex.
    This is very simple: just the

    - seven required attributes as below (copyright may be omitted as it is provided in the base class),
    - list of parameters as required (consult DRL-D for the particular recipe)
    - and finally define the implementation class, which we have just written
    """
    # Fill in recipe information
    _name = "metis_lm_basic_reduce"
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
    implementation_class = MetisLmImgBasicReduceImpl
