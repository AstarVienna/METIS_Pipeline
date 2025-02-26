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

from pymetis.products.product import PipelineProduct
from pymetis.products.common import TargetSpecificProduct
from pymetis.base.recipe import MetisRecipe
from pymetis.inputs import RawInput, BadpixMapInput, PipelineInputSet
from pymetis.inputs.common import OptionalInputMixin, LinearityInput, GainMapInput, MasterFlatInput, MasterDarkInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor


class MetisNImgChopnodImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector: str = "GEO"

        class RawInput(RawInput):
            _tags = re.compile(r"N_IMAGE_(?P<target>SCI|STD)_RAW")
            _description = "Flat field image taken with lamp / sky."

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _tags = re.compile(r"BADPIX_MAP_GEO")

        MasterDarkInput = MasterDarkInput
        MasterFlatInput = MasterFlatInput
        LinearityInput = LinearityInput
        GainMapInput = GainMapInput

    class ProductBkgSubtracted(TargetSpecificProduct):
        band: str = "N"
        _target: str = "target"
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        level: cpl.ui.Frame.FrameLevel = cpl.ui.Frame.FrameLevel.FINAL
        _description = "Thermal background subtracted images of standard N exposures."
        _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

        @classmethod
        def tag(cls) -> str:
            return rf"N_{cls.target():s}_BKG_SUBTRACTED"


    def process_images(self) -> [PipelineProduct]:
        combined_image = self._create_dummy_image()

        product_calibrated = self.ProductBkgSubtracted(self, self.header, combined_image)

        return [product_calibrated]


class MetisNImgChopnodSciImpl(MetisNImgChopnodImpl):
    class ProductBkgSubtracted(MetisNImgChopnodImpl.ProductBkgSubtracted):
        _target: str = "SCI"


class MetisNImgChopnodStdImpl(MetisNImgChopnodImpl):
    class ProductBkgSubtracted(MetisNImgChopnodImpl.ProductBkgSubtracted):
        _target: str = "STD"


class MetisNImgChopnod(MetisRecipe):
    _name: str = "metis_n_img_chopnod"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Chop / nod combination of exposures for background subtraction"

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.FILTER'}
    _algorithm: str = """Analyse and optionally remove masked regions and correct crosstalk and ghosts
    Add/subtract images to subtract background"""

    implementation_class = MetisNImgChopnodImpl

    def dispatch_implementation_class(self, inputset: PipelineInputSet) -> type["MetisRecipeImpl"]:
        return {
            'SCI': MetisNImgChopnodSciImpl,
            'STD': MetisNImgChopnodStdImpl,
        }[inputset.target]
