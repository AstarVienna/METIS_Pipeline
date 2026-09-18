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


class TestInputOrderAndPrimaryFrame:
    def test_inputs_keep_their_declaration_order(self, tmp_path):
        from pymetis.instruments.metis.recipes.ifu.metis_ifu_rsrf import MetisIfuRsrfImpl
        inputset = MetisIfuRsrfImpl.InputSet(frameset(tmp_path, 'IFU_WCU_OFF_RAW', 'IFU_RSRF_RAW'))
        declared = [name for name, _ in MetisIfuRsrfImpl.InputSet.list_input_classes()]
        assert [next(n for n in declared if getattr(inputset, n) is inp) for inp in inputset.inputs] == declared

    def test_the_product_header_comes_from_the_declared_primary_input(self, tmp_path):
        """ metis_ifu_rsrf has two RAW-role inputs; the SOF lists the WCU OFF frame first,
        but the header must be inherited from the RSRF exposure. """
        from pymetis.instruments.metis.recipes.ifu.metis_ifu_rsrf import MetisIfuRsrfImpl
        inputset = MetisIfuRsrfImpl.InputSet(frameset(tmp_path, 'IFU_WCU_OFF_RAW', 'IFU_RSRF_RAW'))
        assert inputset.primary_frame.tag == 'IFU_RSRF_RAW'

    def test_raw_role_inputs_lead_the_used_frames_whatever_the_declaration_order(self, tmp_path):
        """ CPL takes MJD-OBS, DATE-OBS and the RAW1 provenance from the first RAW frame of
        the used-frames list, so the RAW-role inputs must come first even when a
        calibration is declared before them. """
        from pymetis.engine.inputs import PipelineInputSet, PrimaryInputMixin, SinglePipelineInput
        from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg
        from pymetis.instruments.metis.dataitems.img.basicreduced import LmSciBasicReduced

        class Probe(PipelineInputSet):
            class DarkInput(SinglePipelineInput):
                Item = MasterDark2rg

            class ReducedInput(PrimaryInputMixin, SinglePipelineInput):
                Item = LmSciBasicReduced

            dark: DarkInput
            reduced: ReducedInput

        inputset = Probe(frameset(tmp_path, 'MASTER_DARK_2RG', 'LM_SCI_BASIC_REDUCED'))
        assert [type(inp).__name__ for inp in inputset.inputs] == ['DarkInput', 'ReducedInput']
        assert [type(inp).__name__ for inp in inputset.inputs_by_role()] == ['ReducedInput', 'DarkInput']
        assert inputset.primary_frame.tag == 'LM_SCI_BASIC_REDUCED'

    def test_no_raw_role_input_means_no_primary_frame(self, tmp_path):
        from pymetis.engine.inputs import PipelineInputSet

        class Probe(PipelineInputSet):
            dark: MasterDarkInput

        assert Probe(frameset(tmp_path, 'MASTER_DARK_2RG')).primary_frame is None


class TestUseBeforeLoading:
    def test_use_is_safe_before_the_item_is_loaded(self, tmp_path):
        inp = MasterDarkInput(frameset(tmp_path, 'MASTER_DARK_2RG'))
        assert inp.use() is inp and inp._use_requested

    def test_use_is_safe_on_a_multiple_input_before_loading(self, tmp_path):
        from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl
        inp = MetisDetDarkImpl.InputSet.RawInput(frameset(tmp_path, 'DARK_2RG_RAW', 'DARK_2RG_RAW'))
        assert inp.use() is inp and inp._use_requested


class TestTagAxesMustBeDeclared:
    def test_tag_keywords_without_declared_axes_are_refused(self):
        from pymetis.engine.core.parametrizable import Parametrizable
        declared = Parametrizable._valid_tags
        Parametrizable._valid_tags = frozenset()
        try:
            with pytest.raises(TypeError, match="no tag axes are declared"):
                class Probe(Parametrizable, band='LM'):   # noqa: F841
                    pass
        finally:
            Parametrizable._valid_tags = declared


class TestMostSpecificInputWins:
    """ An input declared with a template item matches all its leaves, but a leaf that a
    more specific sibling input declares belongs to that sibling. The SKY leaves are the
    case: `IfuSkyRaw` under `IfuRaw`, `LmSkyBasicReduced` under `BasicReduced`. """

    @staticmethod
    def frameset(directory, *tags):
        frames = cpl.ui.FrameSet()
        for i, tag in enumerate(tags):
            filename = str(directory / f"{tag.lower()}_{i}.fits")
            cpl.core.PropertyList().save(filename, cpl.core.io.CREATE)
            frames.append(cpl.ui.Frame(filename, tag=tag))
        return frames

    def test_sky_frames_go_to_the_sky_input_of_the_ifu_reduce_recipe(self, tmp_path):
        from pymetis.instruments.metis.recipes.ifu.metis_ifu_reduce import MetisIfuReduceImpl
        from pymetis.instruments.metis.dataitems.ifu.raw import IfuSciRaw, IfuSkyRaw
        frames = self.frameset(tmp_path, 'IFU_SCI_RAW', 'IFU_SCI_RAW', 'IFU_SKY_RAW')
        inputset = MetisIfuReduceImpl.InputSet(frames)
        assert inputset.raw.Item is IfuSciRaw
        assert len(inputset.raw.frameset) == 2
        assert inputset.raw_sky.Item is IfuSkyRaw
        assert len(inputset.raw_sky.frameset) == 1

    def test_sky_basic_reduced_goes_to_the_sky_input_of_the_background_recipe(self, tmp_path):
        from pymetis.instruments.metis.recipes.lm_img.metis_lm_img_background import MetisLmImgBackgroundImpl
        from pymetis.instruments.metis.dataitems.img.basicreduced import LmSciBasicReduced, LmSkyBasicReduced
        frames = self.frameset(tmp_path, 'LM_SCI_BASIC_REDUCED', 'LM_SKY_BASIC_REDUCED')
        inputset = MetisLmImgBackgroundImpl.InputSet(frames)
        assert inputset.basic_reduced.Item is LmSciBasicReduced
        assert inputset.sky_basic_reduced.Item is LmSkyBasicReduced
        inputset.validate()

    def test_two_leaves_without_a_claiming_sibling_are_still_ambiguous(self, tmp_path):
        from pymetis.instruments.metis.recipes.lm_img.metis_lm_img_background import MetisLmImgBackgroundImpl
        frames = self.frameset(tmp_path, 'LM_SCI_BASIC_REDUCED', 'LM_STD_BASIC_REDUCED', 'LM_SKY_BASIC_REDUCED')
        with pytest.raises(cpl.core.IllegalInputError, match='several different data items'):
            MetisLmImgBackgroundImpl.InputSet(frames)
