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

from pymetis.base.recipe import MetisRecipe
from pymetis.prefab.img_calibrate import MetisImgCalibrateImpl


class MetisLmImgCalibrateImpl(MetisImgCalibrateImpl):
    class InputSet(MetisImgCalibrateImpl.InputSet):
        class BackgroundInput(MetisImgCalibrateImpl.InputSet.BackgroundInput):
            _tags: re.Pattern = re.compile(r"LM_SCI_BKG_SUBTRACTED")

        class DistortionTableInput(MetisImgCalibrateImpl.InputSet.DistortionTableInput):
            _tags: re.Pattern = re.compile(r"LM_DISTORTION_TABLE")

    class ProductSciCalibrated(MetisImgCalibrateImpl.ProductSciCalibrated):
        _tag = r"LM_SCI_CALIBRATED"
        description = "LM band image with flux calibration, WC coordinate system and distorion information"


class MetisLmImgCalibrate(MetisRecipe):
    _name: str = "metis_lm_img_calibrate"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine optical distortion coefficients for the LM imager."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: {str} = {'DRS.FILTER'}
    _algorithm: str = """Call metis_lm_scale_image_flux to scale image data to photon / s
    Add header information (BUNIT, WCS, etc.)"""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgCalibrateImpl
