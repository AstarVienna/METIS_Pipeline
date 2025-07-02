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

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.mixins.band import BandLmMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl, ABC):
    class InputSet(BandLmMixin, MetisBaseImgFlatImpl.InputSet):
        pass


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = "A*"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master flat for L/M band detectors"
    _description = "Prototype to create a METIS Masterflat for L/M band"

    _matched_keywords = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """For internal flats: call metis_det_dark with LAMP OFF images to create dark frame.
    Subtract internal dark or master dark from flat exposures.
    Call `metis_lm_img_flat` to fit slope of pixel values against illumination level.
    Frames with the same exposure time will be averaged.
    Compute median or average of input frames to improve statistics.
    Call `metis_update_lm_flat_mask` to flag deviant pixels."""

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])

    implementation_class = MetisLmImgFlatImpl
