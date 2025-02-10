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


@pytest.fixture
def name():
    return 'metis_pupil_imaging'


@pytest.fixture
def sof():
    return 'metis_pupil_imaging.lm.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    #@pytest.mark.xfail(reason="SOF file has no master dark yet")
    @pytest.mark.parametrize("sof", ["metis_pupil_imaging.lm.sof", "metis_pupil_imaging.n.sof"])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex):
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)

    #@pytest.mark.xfail(reason="SOF file has no master dark yet")
    @pytest.mark.parametrize("sof", ["metis_pupil_imaging.lm.sof", "metis_pupil_imaging.n.sof"])
    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_recipe_can_be_run_directly(frameset)

    #@pytest.mark.xfail(reason="SOF file has no master dark yet")
    @pytest.mark.parametrize("sof", ["metis_pupil_imaging.lm.sof", "metis_pupil_imaging.n.sof"])
    def test_recipe_uses_all_input_frames(self, load_frameset, sof):
        frameset = load_frameset(sof)
        super().test_recipe_uses_all_input_frames(frameset)


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProduct(BaseProductTest):
    product = Impl.Product

