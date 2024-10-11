import inspect
import subprocess

import cpl
import pytest

from prototypes.product import PipelineProduct
from prototypes.inputs.inputset import PipelineInputSet
from prototypes.recipes.metis_det_dark import MetisDetDark as Recipe, MetisDetDarkImpl as Impl

from prototypes.tests.generic import sof

@pytest.fixture
def masterdark():
    return "prototypes/sof/masterdark.sof"


class TestRecipe:
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_direct(self, sof, masterdark):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(sof(masterdark))
        instance.run(frameset, {})

    def test_pyesorex(self):
        output = subprocess.run(['pyesorex', 'metis_det_dark', 'prototypes/sof/masterdark.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  0  MASTER_DARK_2RG.fits  	MASTER_DARK_2RG  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

    def test_parameter_count(self):
        assert len(Recipe.parameters) == 1


class TestInput:
    def test_is_input(self):
        assert issubclass(Impl.InputSet, PipelineInputSet)

    def test_is_concrete(self):
        assert not inspect.isabstract(Impl.InputSet)

    def test_can_load(self, sof, masterdark):
        instance = Impl.InputSet(sof(masterdark))
        assert instance.verify() is None
        assert len(instance.raw.frameset) == 5



class TestProduct:
    def test_product(self):
        assert issubclass(Impl.Product, PipelineProduct)
