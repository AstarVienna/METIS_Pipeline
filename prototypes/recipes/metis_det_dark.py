from typing import Dict, Any, Literal
from schema import Schema

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.inputs.base import MultiplePipelineInput
from prototypes.inputs.raw import raw_input
from prototypes.product import PipelineProduct
from prototypes.inputs import PipelineInputSet
from prototypes.rawimage import RawImageProcessor

from prototypes.mixins.detectors import Detector2rgMixin


class MetisDetDarkImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class_raw = raw_input(tags=["DARK_{det}_RAW"], det="LM")

        def __init__(self, frameset):
            self.raw = self.class_raw(frameset)
            self.inputs = [self.raw]
            super().__init__(frameset)

    class Product(PipelineProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        def __init__(self,
                     recipe: 'MetisRecipeImpl',
                     header: cpl.core.PropertyList,
                     image: cpl.core.Image,
                     *,
                     detector_name: str, **kwargs):
            self.detector_name = detector_name
            super().__init__(recipe, header, image, **kwargs)

        @property
        def category(self) -> str:
            return rf"MASTER_DARK_{self.detector_name}"

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

    def load_raw_images(self) -> cpl.core.ImageList:
        """
        Always load a set of raw images, as determined by the tags.
        Chi-Hung has warned Martin that this is unnecessary and fills the memory quickly,
        but if we are to use CPL functions, Martin does not see a way around it.
        """
        output = cpl.core.ImageList()
        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.__class__.__qualname__, f"Processing input frame #{idx}: {frame.file!r}...")

            # Append the loaded image to an image list
            Msg.debug(self.__class__.__qualname__, f"Loading input image {frame.file}")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output

    @classmethod
    def combine_images(cls,
                       images: cpl.core.ImageList,
                       method: Literal['add'] | Literal['average'] | Literal['median']):
        """
        Basic helper method to combine images using one of `add`, `average` or `median`.
        Probably not a universal panacea, but it recurs often enough to warrant being here.
        """
        Msg.info(cls.__qualname__, f"Combining images using method {method!r}")
        combined_image = None
        match method:
            case "add":
                for idx, image in enumerate(images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = images.collapse_create()
            case "median":
                combined_image = images.collapse_median_create()
            case _:
                Msg.error(cls.__qualname__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")
                raise ValueError(f"Unknown stacking method {method!r}")

        return combined_image

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
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        return {
            fr'METIS_{self.detector_name}_DARK':
                self.Product(self, header, combined_image,
                             detector_name=self.detector_name),
        }

    @property
    def detector_name(self) -> str | None:
        return "2RG"


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

