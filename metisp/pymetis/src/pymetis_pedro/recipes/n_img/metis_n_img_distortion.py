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

import cpl
from cpl.core import Msg

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem
from pymetis.classes.mixins import BandNMixin
from pymetis.dataitems.distortion.raw import NDistortionRaw
from pymetis.dataitems.raw.wcuoff import NWcuOffRaw
from pymetis.classes.recipes import MetisRecipe
from pymetis.recipes.prefab import MetisBaseImgDistortionImpl
from pymetis.utils.dummy import create_dummy_table


class MetisNImgDistortionImpl(MetisBaseImgDistortionImpl):
    class InputSet(BandNMixin, MetisBaseImgDistortionImpl.InputSet):
        pass

class MetisNImgDistortion(MetisRecipe):
    _name: str = "metis_n_img_distortion"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine optical distortion coefficients for the N imager."

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm: str = """Subtract background image with `hdrl_imagelist_sub_image`.
    Measure location of point source images in frames with `hdrl_catalogue_create`.
    Call metis_fit_distortion to fit polynomial coefficients to deviations from grid positions."""

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisNImgDistortionImpl
