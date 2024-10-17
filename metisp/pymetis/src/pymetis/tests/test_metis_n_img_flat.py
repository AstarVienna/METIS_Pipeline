from pathlib import Path

import pytest
import subprocess
import cpl

from pymetis.recipes.img.metis_n_img_flat import MetisNImgFlat as Recipe, MetisNImgFlatImpl as Impl

from fixtures import create_pyesorex, load_frameset
from generic import BaseInputTest


@pytest.fixture
def sof():
    return Path(__file__).parent.parent.parent.parent / "sof" / "masterflat-n.sof"

@pytest.fixture
def name():
    return 'metis_n_img_flat'


class TestRecipe:
    """ A bunch of extremely simple test cases... just to see if it does something """

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_direct(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_pyesorex(self, create_pyesorex):
        pyesorex = create_pyesorex(Recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_n_img_flat'

    def test_is_working(self, name, sof):
        output = subprocess.run(['pyesorex', name, sof,
                                 '--recipe-dir', 'metisp/pyrecipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        # This is very stupid, but works for now (and more importantly, fails when something's gone wrong)
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_N.fits  	MASTER_IMG_FLAT_LAMP_N  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


class TestInput(BaseInputTest):
    impl = Impl
    count = 1
