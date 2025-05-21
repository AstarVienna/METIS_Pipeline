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

import cpl
from cpl.core import Msg

from pymetis.classes.inputs import RawInput, MasterDarkInput

from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.products import PipelineProduct, BandSpecificProduct, TargetSpecificProduct, PipelineImageProduct


class MetisBaseImgFlatImpl(DarkImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        MasterDarkInput = MasterDarkInput

        class RawInput(RawInput):
            """
            A subclass of RawInput that is handling the flat image raws.
            """
            _tags: re.Pattern = re.compile(r"(?P<band>(LM|N))_FLAT_(?P<target>LAMP|TWILIGHT)_RAW")
            _description: str = "Flat image raw"


    class ProductMasterFlat(BandSpecificProduct, TargetSpecificProduct, PipelineImageProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL

        @classmethod
        def tag(cls) -> str:
            return fr"MASTER_IMG_FLAT_{cls.target():s}_{cls.band():s}"

        @classmethod
        def description(cls) -> str:
            return fr"Master flat frame for {cls.band():s} image data"

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

        product = self.ProductMasterFlat(self, header, combined_image)

        return [product]
