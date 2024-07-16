from typing import Dict, Any
from schema import Schema

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl, MetisRecipe
from prototypes.product import PipelineProduct
from prototypes.rawimage import RawImageProcessor


class MetisDetDarkImpl(RawImageProcessor):
    class Input(RawImageProcessor.Input):
        """ Nothing special """

    class Product(PipelineProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        def __init__(self, recipe: 'Recipe', header: cpl.core.PropertyList, image: cpl.core.Image,
                     *, detector_name: str, **kwargs):
            self.detector_name = detector_name
            super().__init__(recipe, header, image, **kwargs)

        @property
        def category(self) -> str:
            return rf"MASTER_DARK_{self.detector_name}"

        @property
        def output_file_name(self) -> str:
            """ Form the output file name (the detector part is variable here) """
            return rf"MASTER_DARK_{self.detector_name}.fits"

        @property
        def tag(self) -> str:
            return rf"MASTER_DARK_{self.detector_name}"

    def __init__(self, recipe):
        super().__init__(recipe)
        self._detector_name = None

    def verify_input_frames(self) -> None:
        super().verify_input_frames()
        detectors = []

        for idx, frame in enumerate(self.input.raw):
            header = cpl.core.PropertyList.load(frame.file, 0)
            det = header['ESO DPR TECH'].value
            try:
                detector_name = {
                    'IMAGE,LM': '2RG',
                    'IMAGE,N': 'GEO',
                    'IFU': 'IFU'
                }[det]
            except KeyError as e:
                raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e

            detectors.append(detector_name)

        # Check if all the raws have the same detector, if not, we have a problem
        if len(set(detectors)) == 1:
            self._detector_name = detectors[0]
        else:
            raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")

    def process_images(self) -> Dict[str, PipelineProduct]:
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
        raw_images = self.load_input_images()
        combined_image = None

        match method:
            case "add":
                for idx, image in enumerate(raw_images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = raw_images.collapse_create()
            case "median":
                combined_image = raw_images.collapse_median_create()
            case _:
                Msg.error(self.name, f"Got unknown stacking method {method!r}. Stopping right here!")

        header = cpl.core.PropertyList.load(self.input.raw[0].file, 0)

        self.products = {
            fr'METIS_{self.detector_name}_DARK':
                self.Product(self, header, combined_image,
                             detector_name=self.detector_name),
        }

        return self.products

    @property
    def detector_name(self) -> str:
        return self._detector_name


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

