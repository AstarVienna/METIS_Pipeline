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

from pymetis.recipes.lm_lss.metis_lm_lss_sci import (MetisLmLssSci as Recipe,
                                                     MetisLmLssSciImpl as Impl)
from pymetis.tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductTest


recipe_name = r'metis_lm_lss_sci'


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


class TestProductLssSciObjMap(BaseProductTest):
    _product = Impl.ProductLssSciObjMap


class TestProductLssSciSkyMap(BaseProductTest):
    _product = Impl.ProductLssSciSkyMap


class TestProductLssSci1d(BaseProductTest):
    _product = Impl.ProductLssSci1d


class TestProductLssSci2d(BaseProductTest):
    _product = Impl.ProductLssSci2d


class TestProductLssSciFlux1d(BaseProductTest):
    _product = Impl.ProductLssSciFlux1d


class TestProductLssSciFlux2d(BaseProductTest):
    _product = Impl.ProductLssSciFlux2d


class TestProductSciFluxTellCorr1d(BaseProductTest):
    _product = Impl.ProductLssSciFluxTellCorr1d   # TODO: What about the 2d version?
