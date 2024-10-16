import pytest
import subprocess
import cpl

from prototypes.recipes.img.metis_lm_img_flat import MetisLmImgFlat as Recipe, MetisLmImgFlatImpl as Impl

from fixtures import create_pyesorex, load_frameset, BaseInputTest


@pytest.fixture
def sof():
    return "prototypes/sof/masterflat-lm.sof"


class TestRecipe:
    """ A bunch of extremely simple test cases... just to see if it does something """

    def test_create(self):
        recipe = Recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_direct(self, load_frameset, sof):
        instance = Recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_pyesorex(self, create_pyesorex):
        pyesorex = create_pyesorex(Recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == 'metis_lm_img_flat'

    def test_is_working(self, sof):
        output = subprocess.run(['pyesorex', 'metis_lm_img_flat', sof,
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        # This is very stupid, but works for now
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_LM.fits  	MASTER_IMG_FLAT_LAMP_LM  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")


class TestInput(BaseInputTest):
    impl = Impl
    count = 1
