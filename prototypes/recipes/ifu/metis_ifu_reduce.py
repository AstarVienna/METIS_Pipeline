import cpl
from cpl.core import Msg
from typing import Any, Dict, Literal

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.input import PipelineInput
from prototypes.product import PipelineProduct

from prototypes.rawimage import RawImageProcessor
from prototypes.mixins import MasterDarkInputMixin
from prototypes.mixins.detectors import Detector2rgMixin

class MetisIfuReduceImpl(MetisRecipeImpl):
    kind: Literal["SCI"] | Literal["STD"] = None

    class Input(Detector2rgMixin, MasterDarkInputMixin, RawImageProcessor.Input):
        tags_raw = ["IFU_SCI_RAW", "IFU_STD_RAW"]
        tags_dark = ["MASTER_DARK_IFU"]
        tags_wavecal = ["IFU_WAVECAL"]

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.ifu_wavecal: cpl.ui.Frame | None = None
            self.ifu_distortion_table: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case x if x in self.tags_wavecal:
                    self.wave

                    Msg.warning(self.name,
                                f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring it")

        def verify(self):
            self._verify_frame_present(self.ifu_wavecal)
            super().verify()

    class ProductReduced(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.kind}_REDUCED"

    class ProductBackground(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.kind}_BACKGROUND"

    class ProductReducedCube(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.kind}_REDUCED_CUBE"

    class ProductCombined(PipelineProduct):
        @property
        def category(self) -> str:
            return rf"IFU_{self.kind}_COMBINED"


    def process_images(self) -> Dict[str, PipelineProduct]:
        # do something... a lot of something

        self.products = {
            rf'IFU_{self.kind}_REDUCED': self.ProductReduced(),
            rf'IFU_{self.kind}_BACKGROUND': self.ProductBackground(),
            rf'IFU_{self.kind}_REDUCED_CUBE': self.ProductReducedCube(),
            rf'IFU_{self.kind}_COMBINED': self.ProductCombined(),
        }
        return self.products

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        super().run(frameset, settings)

        # TODO: Detect detector
        # TODO: Twilight

        for idx, frame in enumerate(self.input_frames):
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


    @property
    def detector_name(self) -> str:
        """ All IFU recipes work with the HAWAII2RG array """
        return "2RG"

    @property
    def output_file_name(self) -> str:
        return f"IFU_SCI_REDUCED"


class MetisIfuReduce(MetisRecipe):
    _name = "metis_ifu_reduce"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
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
