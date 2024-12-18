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

import pytest
import cpl

from pymetis.recipes.img.metis_n_img_flat import MetisNImgFlat as Recipe, MetisNImgFlatImpl as Impl

from generic import BaseInputSetTest, BaseRecipeTest, BaseProductTest


@pytest.fixture
def name():
    return 'metis_n_img_flat'


@pytest.fixture
def sof():
    return 'metis_n_img_flat.lamp.sof'


# ToDo: GEO raws not yet ready
class DisabledTestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple test cases... just to see if it does something """
    _recipe = Recipe


# ToDo: GEO master dark not yet ready
class DisabledTestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProduct(BaseProductTest):
    product = Impl.Product
