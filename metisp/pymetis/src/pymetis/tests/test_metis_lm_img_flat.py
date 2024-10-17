from pathlib import Path

import pytest
import subprocess
import cpl

from pymetis.recipes.img.metis_lm_img_flat import MetisLmImgFlat as Recipe, MetisLmImgFlatImpl as Impl

from fixtures import create_pyesorex, load_frameset
from generic import BaseInputTest, BaseRecipeTest


@pytest.fixture
def sof():
    return Path(__file__).parent.parent.parent.parent / "sof" / "masterflat-lm.sof"

@pytest.fixture
def name():
    return 'metis_lm_img_flat'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple test cases... just to see if it does something """
    _recipe = Recipe

    def test_is_working(self, name, sof):
        output = subprocess.run(['pyesorex', name, sof,
                                 '--recipe-dir', 'metisp/pyrecipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_LM.fits  	MASTER_IMG_FLAT_LAMP_LM  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


class TestInput(BaseInputTest):
    impl = Impl
    count = 1
