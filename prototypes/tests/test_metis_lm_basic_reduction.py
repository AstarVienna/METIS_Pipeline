import pytest

import cpl
from pyesorex.pyesorex import Pyesorex

from prototypes.recipes.img.metis_lm_basic_reduction import MetisLmBasicReduction, MetisLmBasicReduceImpl
from generic import create_pyesorex


class TestMetisLmBasicReduction:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = MetisLmBasicReduction()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_pyesorex(self, create_pyesorex):
        pyesorex = create_pyesorex(MetisLmBasicReduction)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_lm_basic_reduction'
