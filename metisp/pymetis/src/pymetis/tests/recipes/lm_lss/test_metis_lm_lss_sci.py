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
from pymetis.tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductSetTest


recipe_name = r'metis_lm_lss_sci'


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


class TestProductSetLssSciObjMap(BaseProductSetTest):
    Product = Impl.ProductSet.LssSciObjMap


class TestProductSetLssSciSkyMap(BaseProductSetTest):
    Product = Impl.ProductSet.LssSciSkyMap


class TestProductSetLssSci1D(BaseProductSetTest):
    Product = Impl.ProductSet.LssSci1d


class TestProductSetLssSci2D(BaseProductSetTest):
    Product = Impl.ProductSet.LssSci2d


class TestProductSetLssSciFlux1D(BaseProductSetTest):
    Product = Impl.ProductSet.LssSciFlux1d


class TestProductSetLssSciFlux2D(BaseProductSetTest):
    Product = Impl.ProductSet.LssSciFlux2d


class TestProductSetSciFluxTellCorr1D(BaseProductSetTest):
    Product = Impl.ProductSet.LssSciFluxTellCorr1d   # TODO: What about the 2d version?
