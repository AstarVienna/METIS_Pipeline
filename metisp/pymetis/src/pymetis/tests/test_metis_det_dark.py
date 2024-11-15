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

import os.path
import subprocess
from pathlib import Path

import cpl
import pytest

from pymetis.base.product import PipelineProduct
from pymetis.recipes.metis_det_dark import MetisDetDark as Recipe, Metis2rgDarkImpl as Impl

from generic import BaseInputSetTest, BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_det_dark'


@pytest.fixture
def sof():
    return "metis_det_dark.lm.sof"


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    def test_can_be_created(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_can_be_run_directly(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_parameter_count(self):
        assert len(Recipe.parameters) == 1


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProduct:
    def test_product(self):
        assert issubclass(Impl.Product, PipelineProduct)
