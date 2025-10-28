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

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.coadd import LmSciCoadd
from pymetis.dataitems.img.basicreduced import LmSciCalibrated
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import RawInput
from pymetis.utils.dummy import create_dummy_header


class MetisLmImgSciPostProcessImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LmSciCalibrated

    ProductLmImgSciCoadd = LmSciCoadd

    def process(self) -> set[DataItem]:
        raw_images = self.inputset.raw.load_data('DET1.DATA')
        combined_image = self.combine_images(raw_images, "average")

        primary_header = self.inputset.raw.items[0].primary_header
        header_combined = create_dummy_header()

        product_coadd = self.ProductLmImgSciCoadd(
            primary_header,
            Hdu(header_combined, combined_image, name='IMAGE')
        )

        return {product_coadd}


class MetisLmImgSciPostProcess(MetisRecipe):
    _name: str = "metis_lm_img_sci_postprocess"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Coadd reduced images"

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm = """Check and refine WCS of input images by using the WFS-FS data.
    Determine output pixel grid encompassing all input images.
    Call hdrl_resample_compute to recenter the images.
    Call hdrl_imagelist_collapse to stack the images."""

    parameters = ParameterList([
        ParameterEnum(
            name="metis_lm_img_sci_postprocess.stacking.method",
            context="metis_lm_img_sci_postprocess",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisLmImgSciPostProcessImpl
