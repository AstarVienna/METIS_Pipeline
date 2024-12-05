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

from pymetis.recipes.ifu.metis_ifu_reduce import (MetisIfuReduce as Recipe, MetisIfuReduceImpl as Impl)
from pymetis.tests.generic import BaseRecipeTest, BaseInputSetTest


@pytest.fixture
def name():
    return 'metis_ifu_reduce'


@pytest.fixture
def sof(name):
    return f'{name}.std.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    @pytest.mark.skip
    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        pass

    @pytest.mark.skip
    def test_can_be_run_with_pyesorex(self, name, create_pyesorex):
        pass

    @staticmethod
    @pytest.mark.skip
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex):
        pass


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1

    @pytest.mark.skip(reason="IFU recipes do not have the required master dark yet")
    def test_can_load_and_verify(self, load_frameset, sof):
        super().test_can_load_and_verify(load_frameset, sof)
