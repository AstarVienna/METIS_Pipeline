"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

import re
from abc import ABC
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.inputs import PipelineInputSet
from pymetis.inputs.common import RawInput, MasterDarkInput

from .darkimage import DarkImageProcessor
from ..base.product import PipelineProduct


class MetisBaseImgFlatImpl(DarkImageProcessor, ABC):
    class InputSet(PipelineInputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        class RawFlatInput(RawInput):
            """
            A subclass of RawInput that is handling the flat image raws.
            """
            _tags = re.compile(r"(?P<band>(LM|N))_FLAT_(?P<target>LAMP|TWILIGHT)_RAW")

        class DarkFlatInput(MasterDarkInput):
            """
            Just a plain MasterDarkInput.
            """
            pass

        def __init__(self, frameset):
            self.raw = self.RawFlatInput(frameset)
            self.master_dark = MasterDarkInput(frameset)

            self.inputs = [self.raw, self.master_dark]

            super().__init__(frameset)


    class Product(PipelineProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        band: str = None

        @property
        def category(self) -> str:
            return fr"MASTER_IMG_FLAT_LAMP_{self.band}"

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
