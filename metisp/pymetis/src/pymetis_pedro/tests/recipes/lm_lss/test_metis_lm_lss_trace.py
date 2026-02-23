"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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
from pymetis.recipes.lm_lss.metis_lm_lss_trace import (MetisLmLssTrace as Recipe,
                                                      MetisLmLssTraceImpl as Impl)
from pymetis.tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductTest


recipe_name = r'metis_lm_lss_trace'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.sof'


class TestRecipe(BaseRecipeTest):
    Recipe = Recipe


class TestInputSet(BaseInputSetTest):
    Impl = Impl


class TestProduct(BaseProductTest):
    Product = Impl.ProductTraceTable

