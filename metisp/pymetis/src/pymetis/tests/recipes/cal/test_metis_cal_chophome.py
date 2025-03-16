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
from pymetis.recipes.cal.metis_cal_chophome import (MetisCalChophome as Recipe,
                                                    MetisCalChophomeImpl as Impl)
from pymetis.classes.products import PipelineProduct
from pymetis.tests.classes import BaseInputSetTest, BaseRecipeTest, BaseProductTest


recipe_name = r'metis_cal_chophome'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple test cases... just to see if it does something """
    _recipe: type[MetisRecipe] = Recipe


class TestInputSet(BaseInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl


class TestProductCombined(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductCombined


class TestProductBackground(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductBackground
