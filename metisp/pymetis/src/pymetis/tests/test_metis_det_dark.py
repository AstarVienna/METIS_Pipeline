import subprocess
from pathlib import Path

import cpl
import pytest

from pymetis.base.product import PipelineProduct
from pymetis.recipes.metis_det_dark import MetisDetDark as Recipe, MetisDetDarkImpl as Impl

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

    def test_can_be_run_with_pyesorex(self, name, sof):
        last_line = self.run_with_pyesorex(name, sof)
        assert last_line == ("  0  MASTER_DARK_2RG.fits  	MASTER_DARK_2RG  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

    def test_parameter_count(self):
        assert len(Recipe.parameters) == 1


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 5


class TestProduct:
    def test_product(self):
        assert issubclass(Impl.Product, PipelineProduct)
