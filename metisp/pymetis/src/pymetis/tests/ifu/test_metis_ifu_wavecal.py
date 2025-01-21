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

from pymetis.tests.generic import BaseRecipeTest
from pymetis.recipes.ifu.metis_ifu_wavecal import (MetisIfuWavecal as Recipe,
                                                   MetisIfuWavecalImpl as Impl)


@pytest.fixture
def name():
    return 'metis_ifu_wavecal'


@pytest.fixture
def sof():
    return 'metis_ifu_wavecal.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

