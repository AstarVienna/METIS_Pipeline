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

from pymetis.base import MetisRecipe, MetisRecipeImpl, PipelineProduct
from pymetis.recipes.ifu.metis_ifu_reduce import (MetisIfuReduce as Recipe,
                                                  MetisIfuReduceImpl as Impl)
from pymetis.tests.classes import BaseInputSetTest, TargetParamRecipeTest, BaseProductTest


recipe_name = r'metis_ifu_reduce'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return f'{name}.std.sof'


class TestRecipe(TargetParamRecipeTest):
    _recipe: type[MetisRecipe] = Recipe


class TestInputSet(BaseInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl
    

class TestProductReduced(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductReduced

class TestProductBackground(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductBackground

class TestProductReducedCube(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductReducedCube

class TestProductCombined(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductCombined
