import inspect
import subprocess

import cpl

from prototypes.base import MetisRecipeImpl
from prototypes.product import PipelineProduct
from prototypes.recipes.metis_det_dark import MetisDetDark, MetisDetDarkImpl


class TestRecipe:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = MetisDetDark()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_is_working(self):
        output = subprocess.run(['pyesorex', 'metis_det_dark', 'prototypes/sof/masterdark.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  0  MASTER_DARK_2RG.fits  	MASTER_DARK_2RG  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

    def test_parameter_count(self):
        assert len(MetisDetDark.parameters) == 1


class TestProduct:
    def test_product(self):
        assert issubclass(MetisDetDarkImpl.Product, PipelineProduct)


class TestInput:
    def test_is_input(self):
        assert issubclass(MetisDetDarkImpl.Input, MetisRecipeImpl.Input)

    def test_is_concrete(self):
        assert not inspect.isabstract(MetisDetDarkImpl.Input)
