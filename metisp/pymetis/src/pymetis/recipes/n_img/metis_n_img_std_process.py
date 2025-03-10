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

from pymetis.classes.mixins.band import BandNMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.img_std_process import MetisImgStdProcessImpl
from pymetis.classes.headers.header import Header, HeaderDrsFilter


class MetisNImgStdProcessImpl(MetisImgStdProcessImpl):
    class InputSet(MetisImgStdProcessImpl.InputSet):
        class RawInput(MetisImgStdProcessImpl.InputSet.RawInput):
            _tags: re.Pattern = re.compile(r"N_STD_BKG_SUBTRACTED")
            _description: str = "Thermal background subtracted images of standard N exposures."

    class ProductImgStdCombined(BandNMixin, MetisImgStdProcessImpl.ProductImgStdCombined):
        pass


class MetisNImgStdProcess(MetisRecipe):
    # FixMe This can be probably also largely deduplicated
    _name: str = "metis_n_img_std_process"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine the conversion factor between detector counts and physical source flux"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: {Header} = {HeaderDrsFilter}
    _algorithm: str = """Call metis_n_calculate_std_flux to measure flux in input images
        call hdrl_resample_compute to recenter the images
        call hdrl_imagelist_collapse to stack the images
        call metis_n_calculate_std_flux on the stacked image to get flux of the star in detector units
        call metis_calculate_std_fluxcal to calculate the conversion factor to physical units
        call metis_calculate_detection_limits to compute measure background noise (std, rms) and compute detection limits
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

    implementation_class = MetisNImgStdProcessImpl
