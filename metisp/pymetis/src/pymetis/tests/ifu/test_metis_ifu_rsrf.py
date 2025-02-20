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
from pymetis.recipes.ifu.metis_ifu_rsrf import (MetisIfuRsrf as Recipe,
                                                MetisIfuRsrfImpl as Impl)
from pymetis.tests.generic import BaseRecipeTest, BaseProductTest, RawInputSetTest


recipe_name = r'metis_ifu_rsrf'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return f'{name}.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe: type[MetisRecipe] = Recipe


class TestInputSet(RawInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl


class TestProductRsrfBackground(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductRsrfBackground

class TestProductMasterFlatIfu(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductMasterFlatIfu

class TestProductRsrfIfu(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductRsrfIfu

class TestProductBadpixMap(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductBadpixMapIfu
