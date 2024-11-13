from pathlib import Path

import pytest
import subprocess
import cpl

from pymetis.recipes.img.metis_lm_img_flat import MetisLmImgFlat as Recipe, MetisLmImgFlatImpl as Impl

from generic import BaseInputSetTest, BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_lm_img_flat'


@pytest.fixture
def sof():
    return "metis_lm_img_flat.lamp.sof"

class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple test cases... just to see if it does something """
    _recipe = Recipe

    def test_can_be_run_with_pyesorex(self, name, sof):
        last_line = self.run_with_pyesorex(name, sof)
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_LM.fits  	MASTER_IMG_FLAT_LAMP_LM  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1
