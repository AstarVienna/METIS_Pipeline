import subprocess
from pathlib import Path

import cpl
import pytest

from pymetis.base.product import PipelineProduct
from pymetis.recipes.metis_det_lingain import MetisDetLinGain as Recipe, MetisDetLinGainImpl as Impl

from pymetis.tests.fixtures import load_frameset
from generic import BaseInputSetTest


@pytest.fixture
def sof():
    return Path(__file__).parent.parent.parent.parent / "sof" / "detlin.sof"


class TestRecipe:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_can_be_run_directly(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_can_be_run_with_pyesorex(self, sof):
        output = subprocess.run(['pyesorex', 'metis_det_lingain', sof,
                                 '--recipe-dir', 'metisp/pyrecipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  2  BADPIX_MAP_2RG.fits  	BADPIX_MAP_2RG  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

    def test_parameter_count(self):
        assert len(Recipe.parameters) == 3


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 7


class TestProduct:
    def test_product(self):
        assert issubclass(Impl.Product, PipelineProduct)
