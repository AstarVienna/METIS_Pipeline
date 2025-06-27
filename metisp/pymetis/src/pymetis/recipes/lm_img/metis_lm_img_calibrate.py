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

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.distortion.table import LmDistortionTable
from pymetis.classes.mixins.band import BandLmMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import MetisImgCalibrateImpl


class MetisLmImgCalibrateImpl(MetisImgCalibrateImpl):
    class InputSet(MetisImgCalibrateImpl.InputSet):
        class BackgroundInput(BandLmMixin, MetisImgCalibrateImpl.InputSet.BackgroundInput):
            pass

        class DistortionTableInput(BandLmMixin, MetisImgCalibrateImpl.InputSet.DistortionTableInput):
            Item = LmDistortionTable

    class ProductSciCalibrated(BandLmMixin, MetisImgCalibrateImpl.ProductSciCalibrated):
        pass


class MetisLmImgCalibrate(MetisRecipe):
    _name = "metis_lm_img_calibrate"
    _version = "0.1"
    _author = "Chi-Hung Yan, A*"
    _email = "chyan@asiaa.sinica.edu.tw"
    _synopsis = "Determine optical distortion coefficients for the LM imager."
    _description = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.FILTER'}
    _algorithm = """Call metis_lm_scale_image_flux to scale image data to photon / s
    Add header information (BUNIT, WCS, etc.)"""

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgCalibrateImpl
