from abc import ABC
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.inputs import PipelineInputSet
from pymetis.inputs.common import RawInput, MasterDarkInput
from pymetis.base.product import PipelineProduct

from pymetis.prefabricates.darkimage import DarkImageProcessor


class MetisBaseImgFlatImpl(DarkImageProcessor, ABC):
    class InputSet(PipelineInputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        class RawFlatInput(RawInput):
            _tags = ["{band}_FLAT_LAMP_RAW", "{band}_FLAT_TWILIGHT_RAW"]

        class DarkFlatInput(MasterDarkInput):
            """
            Just the plain MasterDarkInput.
            """
            pass

        def __init__(self, frameset):
            self.raw = self.RawFlatInput(frameset, band=self.band)
            self.master_dark = MasterDarkInput(frameset, det=self.detector)

            self.inputs = [self.raw, self.master_dark]

            super().__init__(frameset)


    class Product(PipelineProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        band: str = None

        @property
        def category(self) -> str:
            return fr"MASTER_IMAGE_FLAT_LAMP_{self.band}"

        @property
        def output_file_name(self) -> str:
            return fr"{self.category}.fits"

        @property
        def tag(self) -> str:
            return self.category

    def process_images(self) -> Dict[str, PipelineProduct]:
        """
        Do the actual processing of the images.
        Here, it means loading the input images and a master dark,
        then subtracting the master dark from every flat,
        and finally combining them into a master flat.
        """
        # TODO: Detect detector
        # TODO: Twilight

        raw_images = self.load_raw_images()
        master_dark = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)

        for raw_image in raw_images:
            Msg.debug(self.__class__.__qualname__, f"Subtracting image {raw_image}")
            raw_image.subtract(master_dark)

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters[f"{self.name}.stacking.method"].value

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not) should come here

        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        combined_image = self.combine_images(self.load_raw_images(), method)

        self.products = {
            self.name.upper(): self.Product(self, header, combined_image),
        }
        return self.products