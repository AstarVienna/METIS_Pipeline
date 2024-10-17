from pathlib import Path

import pytest
import subprocess
import pyesorex
import cpl

from pymetis.recipes.img.metis_lm_basic_reduce import MetisLmBasicReduce as Recipe, MetisLmBasicReduceImpl as Impl
from fixtures import create_pyesorex, load_frameset
from generic import BaseRecipeTest

@pytest.fixture
def sof():
    return Path(__file__).parent.parent.parent.parent / "sof" / "basicsreduction.sof"

@pytest.fixture
def name():
    return 'metis_lm_basic_reduce'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe = Recipe

    def test_is_working(self, name, sof):
        output = subprocess.run(['pyesorex', name, sof,
                                 '--recipe-dir', 'metisp/pyrecipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        # This is very stupid, but works for now (and more importantly, fails when something's gone wrong)
        assert last_line == ("  0  LM_SCI_BASIC_REDUCED.fits  	LM_SCI_BASIC_REDUCED  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

