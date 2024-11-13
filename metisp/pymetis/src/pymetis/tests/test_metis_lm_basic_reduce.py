import pytest

from pymetis.recipes.img.metis_lm_img_basic_reduce import MetisLmImgBasicReduce as Recipe, MetisLmImgBasicReduceImpl as Impl
from generic import BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_lm_basic_reduce'


@pytest.fixture
def sof():
    return 'metis_lm_img_basic_reduce.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    def test_can_be_run_with_pyesorex(self, name, sof):
        last_line = self.run_with_pyesorex(name, sof)
        # This is very stupid, but works for now (and more importantly, fails when something's gone wrong)
        assert last_line == ("  0  LM_SCI_BASIC_REDUCED.fits  	LM_SCI_BASIC_REDUCED  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

