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

from pymetis.recipes.metis_det_dark import MetisDetDark as Recipe, MetisDetDarkImpl as Impl

from generic import BaseInputSetTest, BaseRecipeTest, BaseProductTest


@pytest.fixture
def name():
    return 'metis_det_dark'


@pytest.fixture
def sof():
    return f"metis_det_dark.lm.sof"


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe
    count = 1

    def test_fails_with_inconsistent_tags(self, name):
        self._run_pyesorex(name, "incorrect/metis_det_dark.lm.mixed_detectors.sof")


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProduct(BaseProductTest):
    product = Impl.Product
