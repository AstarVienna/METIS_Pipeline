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
from pymetis.recipes.instrument.metis_pupil_imaging import (MetisPupilImaging as Recipe,
                                                            MetisPupilImagingImpl as Impl)
from pymetis.tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductTest


recipe_name = r'metis_pupil_imaging'
bands = ['lm', 'n']


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.lm.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in bands])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex):
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in bands])
    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_recipe_can_be_run_directly(frameset)

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in bands])
    def test_uses_all_input_frames(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_uses_all_input_frames(frameset)


class TestInputSet(BaseInputSetTest):
    _impl = Impl


class TestProduct(BaseProductTest):
    Product = Impl.ProductReduced
