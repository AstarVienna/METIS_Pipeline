import pytest
import subprocess
import inspect
import cpl
from pyesorex.pyesorex import Pyesorex

from prototypes.recipes.img.metis_lm_img_flat import MetisLmImgFlat, MetisLmImgFlatImpl
from prototypes.input import PipelineInput

from prototypes.tests.generic import sof


@pytest.fixture
def pyesorex():
    p = Pyesorex()
    p.recipe = MetisLmImgFlat._name
    return p


class TestRecipe:
    """ A bunch of extremely simple test cases... just to see if it does something """

    def test_create(self):
        recipe = MetisLmImgFlat()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_pyesorex(self, pyesorex):
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_lm_img_flat'

    def test_is_working(self):
        output = subprocess.run(['pyesorex', 'metis_lm_img_flat', 'prototypes/sof/masterflat.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        # This is very stupid, but works for now
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_LM.fits  	MASTER_IMG_FLAT_LAMP_LM  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


class TestInput:
    def test_is_input(self):
        assert issubclass(MetisLmImgFlatImpl.Input, PipelineInput)

    def test_is_concrete(self):
        assert not inspect.isabstract(MetisLmImgFlatImpl.Input)

    def test_can_load(self, sof):
        files = sof("prototypes/sof/masterflat.sof")
        instance = MetisLmImgFlatImpl.Input(files)
        assert instance.verify() is None
        assert len(instance.raw) == 1

