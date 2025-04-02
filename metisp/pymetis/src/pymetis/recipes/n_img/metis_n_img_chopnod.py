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

import cpl

from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.products import PipelineProduct, TargetSpecificProduct
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import (RawInput, BadpixMapInput, LinearityInput, GainMapInput, MasterFlatInput,
                                    MasterDarkInput, PersistenceInputSetMixin, OptionalInputMixin)
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.headers.header import (Header, HeaderProCatg, HeaderInsOpti3Name, HeaderInsOpti9Name, HeaderInsOpti10Name,
                                            HeaderDetDit, HeaderDetNDit, HeaderDrsFilter)


class MetisNImgChopnodImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector: str = "GEO"

        class RawInput(RawInput):
            _tags = re.compile(r"N_IMAGE_(?P<target>SCI|STD)_RAW")
            _description: str = "Flat field image taken with lamp / sky."

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _tags = re.compile(r"BADPIX_MAP_GEO")

        MasterDarkInput = MasterDarkInput
        MasterFlatInput = MasterFlatInput
        LinearityInput = LinearityInput
        GainMapInput = GainMapInput

    class ProductBkgSubtracted(TargetSpecificProduct):
        band: str = "N"
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        level: cpl.ui.Frame.FrameLevel = cpl.ui.Frame.FrameLevel.FINAL
        _description: str = "Thermal background subtracted images of standard N exposures."
        _oca_keywords: {Header} = {HeaderProCatg, HeaderInsOpti3Name, HeaderInsOpti9Name, HeaderInsOpti10Name, HeaderDrsFilter}

        @classmethod
        def tag(cls) -> str:
            return rf"N_{cls.target():s}_BKG_SUBTRACTED"


    def process_images(self) -> [PipelineProduct]:
        combined_image = self._create_dummy_image()

        product_calibrated = self.ProductBkgSubtracted(self, self.header, combined_image)

        return [product_calibrated]

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        return {
            'SCI': MetisNImgChopnodSciImpl,
            'STD': MetisNImgChopnodStdImpl,
        }[self.inputset.target]


class MetisNImgChopnodStdImpl(MetisNImgChopnodImpl):
    class ProductBkgSubtracted(TargetStdMixin, MetisNImgChopnodImpl.ProductBkgSubtracted):
        pass


class MetisNImgChopnodSciImpl(MetisNImgChopnodImpl):
    class ProductBkgSubtracted(TargetSciMixin, MetisNImgChopnodImpl.ProductBkgSubtracted):
        pass


class MetisNImgChopnod(MetisRecipe):
    _name: str = "metis_n_img_chopnod"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Chop / nod combination of exposures for background subtraction"

    _matched_keywords: {Header} = {HeaderDetDit, HeaderDetNDit, HeaderDrsFilter}
    _algorithm: str = """Analyse and optionally remove masked regions and correct crosstalk and ghosts
    Add/subtract images to subtract background"""

    implementation_class = MetisNImgChopnodImpl
