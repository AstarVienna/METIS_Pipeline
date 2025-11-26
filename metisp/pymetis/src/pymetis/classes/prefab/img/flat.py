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

from abc import ABC

import cpl
from cpl.core import Msg

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.masterflat import MasterImgFlat
from pymetis.dataitems.masterflat.raw import FlatRaw
from pymetis.classes.inputs import RawInput, MasterDarkInput

from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.utils.dummy import create_dummy_header


class MetisBaseImgFlatImpl(DarkImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, DarkImageProcessor.InputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        class MasterDarkInput(MasterDarkInput):
            pass

        class RawInput(RawInput):
            Item = FlatRaw

    ProductMasterFlat = MasterImgFlat

    def process(self) -> set[DataItem]:
        """
        Do the actual processing of the images.
        Here, it means loading the input images and a master dark,
        then subtracting the master dark from every flat,
        and finally combining them into a master flat.
        """
        # TODO: Detect detector
        # TODO: Twilight

        # target = self.inputset.tag_parameters['target']

        self.inputset.raw.load_structure()
        raw_images = self.inputset.raw.load_data('DET1.DATA')
        dark_corrected = self.subtract_dark(raw_images)

        # FixMe: At skeleton level we just copy the header from the first raw
        primary_header = self.inputset.raw.items[0].primary_header

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters[f"{self.name}.stacking.method"].value

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not) should come here
        # ToDo: And this should be moved to the base class anyway
        combined_image = self.combine_images(dark_corrected, method)
        header_master_flat = create_dummy_header()
        # ToDo actually produce the flat

        product = self.ProductMasterFlat(
            primary_header,
            Hdu(header_master_flat, combined_image, name='DET1.SCI'),
        )

        return {product}
