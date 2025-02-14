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

from pymetis.inputs import PipelineInputSet, PersistenceMapInput, LinearityInput
from pymetis.inputs.common import RawInput, MasterDarkInput, BadpixMapInput, GainMapInput

from .darkimage import DarkImageProcessor
from ..base.product import PipelineProduct, BandSpecificProduct, TargetSpecificProduct


class MetisBaseImgFlatImpl(DarkImageProcessor, ABC):
    class InputSet(DarkImageProcessor.InputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        MasterDarkInput = MasterDarkInput

        class RawInput(RawInput):
            """
            A subclass of RawInput that is handling the flat image raws.
            """
            _tags: re.Pattern = re.compile(r"(?P<band>(LM|N))_FLAT_(?P<target>LAMP|TWILIGHT)_RAW")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.persistence = PersistenceMapInput(frameset)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.inputs |= {self.persistence, self.linearity, self.gain_map}

    class Product(BandSpecificProduct, TargetSpecificProduct):
        _group = cpl.ui.Frame.FrameGroup.PRODUCT
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE
        band: str = None
        target: str = None

        @property
        def tag(self) -> str:
            return fr"MASTER_IMG_FLAT_{self.target}_{self.band}"

        @property
        def output_file_name(self) -> str:
            return fr"{self.category}.fits"

    def process_images(self) -> [PipelineProduct]:
        """
        Do the actual processing of the images.
        Here, it means loading the input images and a master dark,
        then subtracting the master dark from every flat,
        and finally combining them into a master flat.
        """
        # TODO: Detect detector
        # TODO: Twilight

        target = self.inputset.tag_parameters['target']

        raw_images = self.inputset.load_raw_images()
        master_dark = cpl.core.Image.load(self.inputset.master_dark.frame.file, extension=0)

        for raw_image in raw_images:
            Msg.debug(self.__class__.__qualname__, f"Subtracting image {raw_image}")
            raw_image.subtract(master_dark)

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters[f"{self.name}.stacking.method"].value

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not) should come here

        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        combined_image = self.combine_images(self.inputset.load_raw_images(), method)

        product = self.Product(self, header, combined_image, target=target)

        return [product]
