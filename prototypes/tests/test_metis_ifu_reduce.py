import pytest

import cpl

from pymetis.recipes.ifu.metis_ifu_reduce import (MetisIfuReduce as Recipe,
                                                     MetisIfuReduceImpl as Impl)
from pymetis.mixins import MasterDarkInputMixin
from generic import create_pyesorex


class TestMetisIfuReduce:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_pyesorex(self, create_pyesorex):
        pyesorex = create_pyesorex(Recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_ifu_reduce'

    def test_mro(self):
        assert MasterDarkInputMixin in Impl.Input.__mro__
