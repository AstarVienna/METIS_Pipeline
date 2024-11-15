import subprocess
from pathlib import Path

import cpl
import pytest

from pymetis.base.product import PipelineProduct
from pymetis.recipes.metis_det_lingain import MetisDetLinGain as Recipe, Metis2rgLinGainImpl as Impl

from generic import BaseInputSetTest, BaseRecipeTest


@pytest.fixture
def name():
    return 'metis_det_lingain'


@pytest.fixture
def sof():
    return 'metis_det_lingain.lm.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_can_be_run_directly(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_parameter_count(self):
        assert len(Recipe.parameters) == 3


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1


class TestProduct:
    def test_product(self):
        assert issubclass(Impl.Product, PipelineProduct)
