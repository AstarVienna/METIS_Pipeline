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

from pymetis.classes.dataitems.raw.flat import LmFlatLampRaw, LmFlatTwilightRaw
from pymetis.classes.mixins.band import BandLmMixin
from pymetis.classes.mixins.target import TargetTwilightMixin, TargetLampMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl, ABC):
    class InputSet(BandLmMixin, MetisBaseImgFlatImpl.InputSet):
        pass

    class ProductMasterFlat(BandLmMixin, MetisBaseImgFlatImpl.ProductMasterFlat):
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    def _dispatch_child_class(self) -> type["MetisRecipeImpl"]:
        return {
            'LAMP': MetisLmImgFlatLampImpl,
            'TWILIGHT': MetisLmImgFlatTwilightImpl,
        }[self.inputset.target]


class MetisLmImgFlatTwilightImpl(MetisLmImgFlatImpl):
    class InputSet(MetisLmImgFlatImpl.InputSet):
        class RawInput(MetisLmImgFlatImpl.InputSet.RawInput):
            Item = LmFlatTwilightRaw

    class ProductMasterFlat(TargetTwilightMixin, MetisLmImgFlatImpl.ProductMasterFlat):
        pass


class MetisLmImgFlatLampImpl(MetisLmImgFlatImpl):
    class InputSet(MetisLmImgFlatImpl.InputSet):
        class RawInput(MetisLmImgFlatImpl.InputSet.RawInput):
            Item = LmFlatLampRaw

    class ProductMasterFlat(TargetLampMixin, MetisLmImgFlatImpl.ProductMasterFlat):
        pass


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name: str = "metis_lm_img_flat"
    _version: str = "0.1"
    _author: str = "A*"
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Create master flat for L/M band detectors"
    _description: str = "Prototype to create a METIS Masterflat for L/M band"

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
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
