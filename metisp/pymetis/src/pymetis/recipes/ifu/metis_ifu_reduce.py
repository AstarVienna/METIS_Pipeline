import cpl
from cpl.core import Msg
from typing import Any, Dict, Literal

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput
from pymetis.inputs.common import RawInput, MasterDarkInput, LinearityInput, PersistenceMapInput

from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuReduceImpl(MetisRecipeImpl):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(DarkImageProcessor.InputSet):
        """
            The Input class for Metis IFU reduction. Utilizes InputMixins:

            - Detector2rgMixin, which handles the 2RG detector and substitudes '2RG' for 'det' in tags
            - LinearityInputMixin, which
        """
        detector = "IFU"

        class RawInput(RawInput):
            _tags = ["IFU_SCI_RAW", "IFU_STD_RAW"]


        # We know which files to handle and how, but we need to specify how to identify them: define tags_something
        # for every mixin and the class itself.
        tags_raw = ["IFU_SCI_RAW", "IFU_STD_RAW"]
        tags_dark = ["MASTER_DARK_IFU"]
        tags_wavecal = ["IFU_WAVECAL"]

        def __init__(self, frameset: cpl.ui.FrameSet):
            """
                Here we also define all input frames specific for this recipe, except those handled by mixins.
            """
            self.raw = RawInput(frameset, det=self.detector)
            self.linearity_map = LinearityInput(frameset)
            self.persistence_map = PersistenceMapInput(frameset)
            self.master_dark = MasterDarkInput(frameset, det="IFU")
            self.ifu_wavecal = SinglePipelineInput(frameset, tags=["IFU_WAVECAL"])
            self.ifu_distortion_table = SinglePipelineInput(frameset, tags=["IFU_DISTORTION_TABLE"])
            super().__init__(frameset)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case x if x in self.tags_wavecal:
                    self.ifu_wavecal = frame
                    Msg.debug(self.__class__.__qualname__,
                              f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring it")

        def verify(self):
            """
                During verification, we see if there is the correct number of frames.
                Note that mixins and parent methods are called last.
            """
            self._verify_frame_present(self.ifu_wavecal, "IFU wavelength calibration")
            super().verify()

    class ProductReduced(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.target}_REDUCED"

    class ProductBackground(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.target}_BACKGROUND"

    class ProductReducedCube(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.target}_REDUCED_CUBE"

    class ProductCombined(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.target}_COMBINED"


    def process_images(self) -> Dict[str, PipelineProduct]:
        # do something... a lot of something

        self.products = {
            rf'IFU_{self.target}_REDUCED': self.ProductReduced(),
            rf'IFU_{self.target}_BACKGROUND': self.ProductBackground(),
            rf'IFU_{self.target}_REDUCED_CUBE': self.ProductReducedCube(),
            rf'IFU_{self.target}_COMBINED': self.ProductCombined(),
        }
        return self.products

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """
        This is obsolete: move functionality to process_iamges
        and abolish this function (it is calle dbut from root class)
        """
        for idx, frame in enumerate(self.raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            # Subtract dark
            raw_image.subtract(masterdark_image)

            # Insert the processed image in an image list.
            # Of course there is also an append() method available.
            raw_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.

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


class MetisIfuReduce(MetisRecipe):
    _name = "metis_ifu_reduce"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_reduce.telluric",
            context="metis_ifu_reduce",
            description="IFU basic data reduction",
            default=False,
            alternatives=(True, False),
        ),
    ])
    implementation_class = MetisIfuReduceImpl
