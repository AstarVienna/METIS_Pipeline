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
from pymetis.prefab.flat import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band: str = "LM"
        detector: str = "2RG"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "LM"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = ["Kieran Leschinski", "Chi-Hung Yan", "Hugo Buddelmeijer", "Gilles PPL Otten", "Martin Baláž"]
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master flat for L/M band detectors"
    _description = "Prototype to create a METIS Masterflat for L/M band"

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
