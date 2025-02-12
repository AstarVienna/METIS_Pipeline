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

from pymetis.tests.conftest import load_frameset
from pymetis.tests.generic import BaseRecipeTest, BaseInputSetTest, BaseProductTest
from pymetis.recipes.instrument.metis_pupil_imaging import (MetisPupilImaging as Recipe,
                                                            MetisPupilImagingImpl as Impl)


recipe_name = r'metis_pupil_imaging'

@pytest.fixture
def name():
    return recipe_name


@pytest.fixture
def sof(name):
    return rf'{name}.lm.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in ['lm', 'n']])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex):
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in ['lm', 'n']])
    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_recipe_can_be_run_directly(frameset)

    @pytest.mark.parametrize("sof", [f"{recipe_name}.{band}.sof" for band in ['lm', 'n']])
    def test_recipe_uses_all_input_frames(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_recipe_uses_all_input_frames(frameset)


class TestInputSet(BaseInputSetTest):
    impl = Impl


class TestProduct(BaseProductTest):
    product = Impl.Product

