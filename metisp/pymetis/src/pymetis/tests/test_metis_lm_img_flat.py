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


class TestInputSet(BaseInputSetTest):
    impl = Impl
    count = 1
