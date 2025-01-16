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

from pymetis.recipes.ifu.metis_ifu_postprocess import (MetisIfuPostprocess as Recipe, MetisIfuPostprocessImpl as Impl)
from pymetis.tests.generic import BaseRecipeTest, BaseInputSetTest, BaseProductTest


@pytest.fixture
def name():
    return 'metis_ifu_postprocess'


@pytest.fixture
def sof(name):
    return f'{name}.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    @pytest.mark.xfail
    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        super().test_recipe_can_be_run_directly(load_frameset, sof)

    @pytest.mark.xfail
    def test_can_be_run_with_pyesorex(self, name, create_pyesorex):
        super().test_can_be_run_with_pyesorex(name, create_pyesorex)

    @staticmethod
    @pytest.mark.xfail
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex):
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)


class DisabledTestInputSet(BaseInputSetTest):
    impl = Impl
