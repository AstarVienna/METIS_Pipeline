"""
Tests for the classification of SOF frames into a `PipelineInput`: promotion to the
specialized data item found in the frames, and the refusal of frames of several
different data items in one input.
"""
import cpl
import pytest
from cpl.core import PropertyList as CplPropertyList

import pymetis.instruments.metis.dataitems  # noqa: F401  (fills the data item registry)
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg, MasterDarkGeo
from pymetis.instruments.metis.inputs.common import MasterDarkInput


def frameset(directory, *tags: str) -> cpl.ui.FrameSet:
    """ One (empty) FITS file per tag; `cpl.ui.Frame` insists that its file exists,
    but classification itself never opens it. """
    frames = cpl.ui.FrameSet()
    for i, tag in enumerate(tags):
        filename = str(directory / f"{tag.lower()}_{i}.fits")
        CplPropertyList().save(filename, cpl.core.io.CREATE)
        frames.append(cpl.ui.Frame(filename, tag=tag))
    return frames


class TestInputClassification:
    def test_a_single_tag_promotes_the_item(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG'))
        assert inp.Item is MasterDark2rg

    def test_unrelated_tags_are_ignored(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_GEO', 'PERSISTENCE_MAP'))
        assert inp.Item is MasterDarkGeo

    def test_frames_of_several_data_items_are_refused(self, tmp_path):
        """ Two detectors' master darks can never feed one input; the old code
        loaded both in turn and silently kept whichever came last. """
        with pytest.raises(cpl.core.IllegalInputError, match='MASTER_DARK_2RG.*MASTER_DARK_GEO'):
            MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG', 'MASTER_DARK_GEO'))


class TestFrameRole:
    """ The CPL frame group stamped on loaded frames is the input's role in the recipe,
    not the item's origin: a pipeline product is RAW to the recipe that reduces it. """

    def test_a_raw_input_stamps_raw(self, tmp_path):
        from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl
        inp = MetisDetDarkImpl.InputSet.RawInput(frameset(tmp_path, 'DARK_2RG_RAW', 'DARK_2RG_RAW'))
        assert {frame.group for frame in inp.frameset} == {cpl.ui.Frame.FrameGroup.RAW}

    def test_a_calibration_input_stamps_calib(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG'))
        assert inp.frame.group == cpl.ui.Frame.FrameGroup.CALIB

    def test_a_product_reduced_by_the_next_recipe_is_its_raw_frame(self, tmp_path):
        from pymetis.instruments.metis.recipes.n_img.metis_n_img_restore import MetisNImgRestoreImpl
        inp = MetisNImgRestoreImpl.InputSet.CalibratedInput(frameset(tmp_path, 'N_SCI_CALIBRATED'))
        assert inp.Item.frame_group() == cpl.ui.Frame.FrameGroup.PRODUCT
        assert inp.frame.group == cpl.ui.Frame.FrameGroup.RAW


class TestValidation:
    def test_a_calibration_of_another_detector_fails_validation_naming_the_expected_tag(self, tmp_path):
        """ The 2RG flat recipe accepts MASTER_DARK_2RG only; a GEO dark in the SOF used
        to be logged and forgotten, and the run died later on an unrelated promotion error. """
        from pymetis.instruments.metis.recipes.lm_img.metis_lm_img_flat import MetisLmImgFlatImpl
        frames = frameset(tmp_path, 'LM_FLAT_LAMP_RAW', 'LM_FLAT_LAMP_RAW', 'MASTER_DARK_GEO',
                          'GAIN_MAP_2RG', 'LINEARITY_2RG')
        inputset = MetisLmImgFlatImpl.InputSet(frames)
        with pytest.raises(cpl.core.DataNotFoundError, match='MASTER_DARK_2RG'):
            inputset.validate()

    def test_a_complete_set_validates_and_collects_the_data_tags(self, tmp_path):
        from pymetis.instruments.metis.recipes.lm_img.metis_lm_img_flat import MetisLmImgFlatImpl
        frames = frameset(tmp_path, 'LM_FLAT_LAMP_RAW', 'LM_FLAT_LAMP_RAW', 'MASTER_DARK_2RG',
                          'GAIN_MAP_2RG', 'LINEARITY_2RG')
        inputset = MetisLmImgFlatImpl.InputSet(frames)
        inputset.validate()
        assert inputset.tag_matches['source'] == 'LAMP'


class TestRecipeTagsAreAuthoritative:
    def test_a_frame_contradicting_the_recipe_tags_is_refused(self, tmp_path):
        """ An N-band recipe handed an LM-tagged item (LmSciBasicReduced carries band=LM): the
        frame matched, as nothing narrowed this input, but the recipe's own band wins. """
        from types import SimpleNamespace
        from pymetis.engine.core.parameter import ParameterList
        from pymetis.engine.inputs import PipelineInputSet, SinglePipelineInput
        from pymetis.engine.recipes import RecipeImpl
        from pymetis.instruments.metis.dataitems.img.basicreduced import LmSciBasicReduced
        from pymetis.instruments.metis.mixins import BandNMixin

        class Probe(BandNMixin, RecipeImpl):
            class InputSet(PipelineInputSet):
                class SkyInput(SinglePipelineInput):
                    Item = LmSciBasicReduced

                sky: SkyInput

            def process(self):
                return set()

        recipe = SimpleNamespace(name='probe', version='0', parameters=ParameterList([]))
        with pytest.raises(cpl.core.IllegalInputError, match="band is 'N' here but 'LM' in the data"):
            Probe(recipe, frameset(tmp_path, 'LM_SCI_BASIC_REDUCED'), {})
