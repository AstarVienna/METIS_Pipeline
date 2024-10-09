from typing import Dict, Any

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.inputs import PipelineInputSet
from prototypes.inputs.raw import raw_input
from prototypes.product import PipelineProduct
from prototypes.rawimage import RawImageProcessor

from prototypes.mixins.detectors import Detector2rgMixin


class MetisDetDarkImpl(RawImageProcessor):
    @property
    def detector_name(self) -> str | None:
        return "2RG"

    class InputSet(PipelineInputSet):
        def __init__(self, frameset: cpl.ui.FrameSet = None, **kwargs):
            self.raw = raw_input(tags=["DARK_{det}_RAW"], det="LM")(frameset)
            self.inputs = [self.raw]
            super().__init__(frameset, **kwargs)

    class Product(PipelineProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        def __init__(self,
                     recipe: 'MetisRecipeImpl',
                     header: cpl.core.PropertyList,
                     image: cpl.core.Image,
                     *,
                     detector: str, **kwargs):
            self.detector = detector
            super().__init__(recipe, header, image, **kwargs)

        @property
        def category(self) -> str:
            return rf"MASTER_DARK_{self.detector}"

        @property
        def output_file_name(self) -> str:
            """ Form the output file name (the detector part is variable here) """
            return rf"{self.category}.fits"

        @property
        def tag(self) -> str:
            return rf"{self.category}"

    def __init__(self, recipe):
        self._detector_name = None
        super().__init__(recipe)

    def process_images(self) -> Dict[str, PipelineProduct]:
        
        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is, however, also possible to load images without
        # performing this conversion.

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.__class__.__qualname__, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["metis_det_dark.stacking.method"].value
        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        raw_images = self.load_raw_images()
        combined_image = self.combine_images(raw_images, method)
        header = cpl.core.PropertyList.load(self.input.raw.frameset[0].file, 0)

        return {
            fr'METIS_{self.detector_name}_DARK':
                self.Product(self, header, combined_image,
                             detector=self.detector_name),
        }


class Metis2rgDarkImpl(Detector2rgMixin, MetisDetDarkImpl):
    tags_dark = ["DARK_IFU_RAW"]


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

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_det_dark.stacking.method",
            context="metis_det_dark",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisDetDarkImpl

