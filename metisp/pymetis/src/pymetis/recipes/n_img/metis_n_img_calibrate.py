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

from pymetis.classes.dataitems.background.subtracted import NSciBackgroundSubtracted
from pymetis.classes.dataitems.distortion.table import NDistortionTable
from pymetis.classes.mixins.band import BandNMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import MetisImgCalibrateImpl


class MetisNImgCalibrateImpl(MetisImgCalibrateImpl):
    class InputSet(MetisImgCalibrateImpl.InputSet):
        class BackgroundInput(BandNMixin, MetisImgCalibrateImpl.InputSet.BackgroundInput):
            Item = NSciBackgroundSubtracted

        class DistortionTableInput(BandNMixin, MetisImgCalibrateImpl.InputSet.DistortionTableInput):
            Item = NDistortionTable

    class ProductSciCalibrated(BandNMixin, MetisImgCalibrateImpl.ProductSciCalibrated):
        pass


class MetisNImgCalibrate(MetisRecipe):
    _name: str = "metis_n_img_calibrate"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine optical distortion coefficients for the N imager."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm: str = """Call metis_n_scale_image_flux to scale image data to photon / s
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

    implementation_class = MetisNImgCalibrateImpl
