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
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput
from pymetis.inputs.common import FluxstdCatalogInput
from pymetis.prefab.rawimage import RawImageProcessor


class MetisLmImgsStdProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_STD_BKG_SUBTRACTED")
            _description: str = "Thermal background subtracted images of standard LM exposures."

        FluxstdCatalogInput = FluxstdCatalogInput

    class ProductLmImgFluxCalTable(PipelineProduct):
        _tag = r"FLUXCAL_TAB"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE

    class ProductLmImgStdCombined(PipelineProduct):
        _tag = r"LM_STD_COMBINED"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=0)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")

        product_fluxcal = self.ProductLmImgFluxCalTable(self, self.header, combined_image)
        product_combined = self.ProductLmImgStdCombined(self, self.header, combined_image)

        return [product_fluxcal, product_combined]


class MetisLmImgStdProcess(MetisRecipe):
    _name: str = "metis_lm_img_std_process"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine the conversion factor between detector counts and physical source flux"
    _description: str = (
        "Currently just a skeleton prototype."
    )
    _matched_keywords: [str] = ['DRS.FILTER']
    _algorithm = """Call metis_lm_calculate_std_flux to measure flux in input images
        call hdrl_resample_compute to recenter the images
        call hdrl_imagelist_collapse to stack the images
        call metis_lm_calculate_std_flux on the stacked image to get flux of the star in detector units
        call metis_calculate_std_fluxcal to calculate the conversion factor to physical units
        call metis_calculate_detection_limits to compute measure background noise (std,rms) and compute detection limits
    """

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgsStdProcessImpl
