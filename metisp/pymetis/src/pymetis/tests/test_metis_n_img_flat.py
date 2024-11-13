from pathlib import Path

import pytest
import subprocess
import cpl

from pymetis.recipes.img.metis_n_img_flat import MetisNImgFlat as Recipe, MetisNImgFlatImpl as Impl

from fixtures import create_pyesorex, load_frameset
from generic import BaseInputSetTest
from pymetis.tests.generic import BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_n_img_flat'


@pytest.fixture
def sof():
    return 'metis_n_img_flat.lamp.sof'


class TestRecipe(BaseRecipeTest):
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


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1
