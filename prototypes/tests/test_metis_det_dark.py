import subprocess
import pytest


class TestMetisDetDark:
    """ An extremely simple and stupid pair of test cases just to see if it does something """

    def test_metis_det_dark(self):
        output = subprocess.run(['pyesorex', 'metis_det_dark', 'prototypes/sof/masterdark.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                 capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  0  MASTER_DARK_2RG.fits  	MASTER_DARK_2RG  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")

    def test_metis_lm_img_flat(self):
        output = subprocess.run(['pyesorex', 'metis_lm_img_flat', 'prototypes/sof/masterflat.sof',
                                 '--recipe-dir', 'prototypes/recipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        last_line = output.stdout.decode('utf-8').split('\n')[-3]
        assert last_line == ("  0  MASTER_IMG_FLAT_LAMP_LM.fits  	MASTER_IMG_FLAT_LAMP_LM  CPL_FRAME_TYPE_IMAGE  "
                             "CPL_FRAME_GROUP_PRODUCT  CPL_FRAME_LEVEL_FINAL  ")
