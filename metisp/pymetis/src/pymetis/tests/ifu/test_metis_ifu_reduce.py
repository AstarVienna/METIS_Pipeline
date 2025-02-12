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

from pymetis.recipes.ifu.metis_ifu_reduce import (MetisIfuReduce as Recipe,
                                                  MetisIfuReduceImpl as Impl)
from pymetis.tests.generic import BaseInputSetTest, TargetParamRecipeTest, BaseProductTest


recipe_name = r'metis_ifu_reduce'


@pytest.fixture
def name():
    return recipe_name


@pytest.fixture
def sof(name):
    return f'{name}.std.sof'


class TestRecipe(TargetParamRecipeTest):
    _recipe = Recipe


class TestInputSet(BaseInputSetTest):
    impl = Impl
    

class TestProductReduced(BaseProductTest):
    product = Impl.ProductReduced

class TestProductBackground(BaseProductTest):
    product = Impl.ProductBackground

class TestProductReducedCube(BaseProductTest):
    product = Impl.ProductReducedCube

class TestProductCombined(BaseProductTest):
    product = Impl.ProductCombined