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

from pathlib import Path

import pytest
import subprocess
import cpl

from pymetis.recipes.img.metis_n_img_flat import MetisNImgFlat as Recipe, MetisNImgFlatImpl as Impl

from generic import BaseInputSetTest
from pymetis.tests.generic import BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_n_img_flat'


@pytest.fixture
def sof():
    return 'metis_n_img_flat.lamp.sof'


# ToDo: disabled for release!
class DisabledTestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple test cases... just to see if it does something """
    _recipe = Recipe

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_can_be_run_directly(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_can_be_run_with_pyesorex(self, name, sof):
        last_line = self.run_with_pyesorex(name, sof)
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_N.fits  	MASTER_IMG_FLAT_LAMP_N  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


# ToDo: disabled for release!
class DisabledTestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1
