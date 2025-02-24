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

from pymetis.base.recipe import MetisRecipe
from pymetis.inputs import PipelineInputSet
from pymetis.prefab.flat import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band: str = "LM"
        detector: str = "2RG"

    class ProductMasterFlat(MetisBaseImgFlatImpl.ProductMasterFlat):
        band: str = "LM"


class MetisLmImgFlatTwilightImpl(MetisLmImgFlatImpl):
    class ProductMasterFlat(MetisLmImgFlatImpl.ProductMasterFlat):
        target: str = "TWILIGHT"


class MetisLmImgFlatLampImpl(MetisLmImgFlatImpl):
    class ProductMasterFlat(MetisLmImgFlatImpl.ProductMasterFlat):
        target: str = "LAMP"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name: str = "metis_lm_img_flat"
    _version: str = "0.1"
    _author: str = ["Kieran Leschinski", "Chi-Hung Yan", "Hugo Buddelmeijer", "Gilles PPL Otten", "Martin Baláž"]
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Create master flat for L/M band detectors"
    _description: str = "Prototype to create a METIS Masterflat for L/M band"

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])
    implementation_class = MetisLmImgFlatImpl

    def dispatch_implementation_class(self, inputset: PipelineInputSet) -> type["MetisRecipeImpl"]:
        return {
            'LAMP': MetisLmImgFlatLampImpl,
            'TWILIGHT': MetisLmImgFlatTwilightImpl,
        }[inputset.target]