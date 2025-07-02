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
from pymetis.recipes.lm_img.metis_lm_img_background import (MetisLmImgBackground as Recipe,
                                                            MetisLmImgBackgroundImpl as Impl)
from pymetis.classes.products import PipelineProduct
from pymetis.tests.classes import BaseInputSetTest, BaseProductTest, TargetParamRecipeTest, BaseRecipeTest


recipe_name = r'metis_lm_img_background'
targets = [r'sci', r'std']


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.std.sof'


class TestRecipe(BaseRecipeTest):
    _recipe: type[MetisRecipe] = Recipe

    @pytest.mark.pyesorex
    @pytest.mark.parametrize("sof", [f"{recipe_name}.{target}.sof" for target in targets])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex):
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)


class TestInputSet(BaseInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl


class TestProductBkg(BaseProductTest):
    Product: type[PipelineProduct] = Impl.ProductBkg


class TestProductBkgSubtracted(BaseProductTest):
    Product: type[PipelineProduct] = Impl.ProductBkgSubtracted


class TestProductObjectCat(BaseProductTest):
    Product: type[PipelineProduct] = Impl.ProductObjectCatalog
