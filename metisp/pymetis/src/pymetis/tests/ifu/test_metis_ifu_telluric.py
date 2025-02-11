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

from pymetis.recipes.ifu.metis_ifu_telluric import (MetisIfuTelluric as Recipe,
                                                    MetisIfuTelluricImpl as Impl)
from pymetis.tests.generic import TargetParamRecipeTest, BaseInputSetTest, BaseProductTest


recipe_name = r'metis_ifu_telluric'


@pytest.fixture
def name():
    return recipe_name


@pytest.fixture
def sof(name):
    return f'{name}.std.sof'


class TestRecipe(TargetParamRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProductFluxcalTab(BaseProductTest):
    product = Impl.ProductFluxcalTab


class TestProductTelluric(BaseProductTest):
    product = Impl.ProductTelluricTransmission


class TestProductResponseFunction(BaseProductTest):
    product = Impl.ProductResponseFunction