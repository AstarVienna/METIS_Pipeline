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
