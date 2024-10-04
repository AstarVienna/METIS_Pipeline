import pytest
import subprocess

import cpl
from pyesorex.pyesorex import Pyesorex

from prototypes.recipes.img.metis_n_img_flat import MetisNImgFlat, MetisNImgFlatImpl
from generic import create_pyesorex



class TestRecipe:
    """ A bunch of extremely simple test cases... just to see if it does something """
    cls = MetisNImgFlat

    def test_create(self):
        recipe = MetisNImgFlat()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_pyesorex(self, create_pyesorex):
        p = create_pyesorex(self.cls)
        assert isinstance(p.recipe, cpl.ui.PyRecipe)
        assert p.recipe.name == 'metis_n_img_flat'

    def test_is_working(self):
        output = subprocess.run(['pyesorex', 'metis_n_img_flat', 'prototypes/sof/masterflat-n.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        # This is very stupid, but works for now (and more importantly, fails when something's gone wrong)
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_N.fits  	MASTER_IMG_FLAT_LAMP_N  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")
