import subprocess

import cpl

from prototypes.recipes.metis_lm_basic_reduction import MetisLmBasicReduction


class TestMetisLmBasicReduction:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = MetisLmBasicReduction()
        assert isinstance(recipe, cpl.ui.PyRecipe)
