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

from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.recipes.metis_det_lingain import (MetisDetLinGain as Recipe,
                                               MetisDetLinGainImpl as Impl)
from pymetis.tests.classes import RawInputSetTest, BandParamRecipeTest
from pymetis.tests.classes.product import ImageProductTest

recipe_name = r'metis_det_lingain'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.ifu.sof'


class TestRecipe(BandParamRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    Recipe = Recipe


class TestInputSet(RawInputSetTest):
    Impl = Impl


class TestProductGain(ImageProductTest):
    Product = Impl.ProductGainMap


class TestProductLinearity(ImageProductTest):
    Product = Impl.ProductLinearity


class TestProductBadPixMap(ImageProductTest):
    Product = Impl.ProductBadPixMap
