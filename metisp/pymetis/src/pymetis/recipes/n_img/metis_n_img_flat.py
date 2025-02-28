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

from pymetis.classes.mixins.target import TargetTwilightMixin, TargetLampMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import MetisBaseImgFlatImpl


class MetisNImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        _band: str = "N"

    class ProductMasterFlat(MetisBaseImgFlatImpl.ProductMasterFlat):
        _band: str = "N"
        _oca_keywords: {str} = {'PRO.CATG', 'DRS.FILTER'}

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        return {
            'LAMP': MetisNImgFlatLampImpl,
            'TWILIGHT': MetisNImgFlatTwilightImpl,
        }[self.inputset.target]


class MetisNImgFlatTwilightImpl(MetisNImgFlatImpl):
    class ProductMasterFlat(TargetTwilightMixin, MetisNImgFlatImpl.ProductMasterFlat):
        pass


class MetisNImgFlatLampImpl(MetisNImgFlatImpl):
    class ProductMasterFlat(TargetLampMixin, MetisNImgFlatImpl.ProductMasterFlat):
        pass


class MetisNImgFlat(MetisRecipe):
    # Fill in recipe information
    _name: str = "metis_n_img_flat"
    _version: str = "0.1"
    _author: str = "Hugo Buddelmeijer, A*"
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Create master flat for N band detectors"
    _description: str = "Prototype to create a METIS master flat for N band"

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.FILTER'}
    _algorithm: str = """For internal flats: call metis_det_dark with LAMP OFF im ages to create dark frame.
    Subtract internal dark or master dark from flat exposures.
    Call metis_n_img_flat to fit slope of pixel values against illumination level.
    Frames with the same exposure time will be averaged.
    Compute median or average of input frames to improve statistics.
    Call metis_update_n_flat_mask to flag deviant pixels."""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])

    implementation_class = MetisNImgFlatImpl
