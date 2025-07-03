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

from pymetis.recipes.n_lss.metis_n_lss_rsrf import (MetisNLssRsrf as Recipe,
                                                    MetisNLssRsrfImpl as Impl)
from pymetis.tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductTest


recipe_name = r'metis_n_lss_rsrf'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.sof'


class TestRecipe(BaseRecipeTest):
    _recipe = Recipe


class TestInputSet(BaseInputSetTest):
    _impl = Impl


class TestProductMedian(BaseProductTest):
    _product = Impl.ProductMedianLssRsrf


class TestProductMean(BaseProductTest):
    _product = Impl.ProductMeanLssRsrf


class TestProductMaster(BaseProductTest):
    _product = Impl.ProductMasterLssRsrf
