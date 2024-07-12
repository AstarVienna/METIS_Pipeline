import subprocess

import cpl

from prototypes.recipes.metis_det_dark import MetisDetDark, MetisDetDarkImpl


class TestMetisDetDark:
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

    def test_metis_lm_img_flat(self):
        assert len(MetisDetDark.parameters) == 1
