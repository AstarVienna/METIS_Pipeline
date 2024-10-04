import pytest

import cpl
from pyesorex.pyesorex import Pyesorex
from prototypes.recipes.img.metis_lm_basic_reduction import MetisLmBasicReduction


@pytest.fixture
def pyesorex():
    p = Pyesorex()
    p.recipe = MetisLmBasicReduction._name
    return p


class TestMetisLmBasicReduction:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = MetisLmBasicReduction()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_pyesorex(self, pyesorex):
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_lm_basic_reduction'
