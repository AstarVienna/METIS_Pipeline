"""
Tests for the explicit, annotation-based declaration of InputSet attributes:
the per-defining-class verification and the annotation-based recipe introspection.
"""
import pytest

from pymetis.engine.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.engine.recipes.recipe import Recipe

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the recipe registry)
from pymetis.instruments.metis.dataitems.distortion.table import DistortionTable
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark
from pymetis.instruments.metis.recipes.ifu.metis_ifu_wavecal import MetisIfuWavecal
from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDark


class AlphaInput(SinglePipelineInput):
    pass


class BetaInput(SinglePipelineInput):
    pass


class ParentInputSet(PipelineInputSet):
    class NestedInput(AlphaInput):
        pass

    alpha: NestedInput


class TestVerifyAllInputsAreDeclared:
    def test_accepts_a_nested_class_bound_in_its_own_body(self):
        ParentInputSet._verify_all_inputs_are_declared()

    def test_accepts_an_override_by_reannotation_alone(self):
        """ The parent's nested class stays behind, bound at the parent's level. """
        class ChildInputSet(ParentInputSet):
            alpha: BetaInput

        ChildInputSet._verify_all_inputs_are_declared()
        assert dict(ChildInputSet.list_input_classes()) == {'alpha': BetaInput}

    def test_rejects_a_nested_class_left_unannotated(self):
        """ Overriding the nested class without re-annotating would silently
        instantiate the inherited annotation's class instead. """
        class ChildInputSet(ParentInputSet):
            class NestedInput(BetaInput):
                pass

        with pytest.raises(TypeError, match="NestedInput"):
            ChildInputSet._verify_all_inputs_are_declared()


class TestManPageTagResolution:
    """
    Input lines of the man page resolve the tag placeholders the recipe pins
    statically through its mixins; tags only the input data can determine
    remain as `{placeholders}`.
    """
    def test_pinned_tags_are_resolved(self):
        """ metis_ifu_wavecal pins the detector through DetectorIfuMixin. """
        description = MetisIfuWavecal._build_description()
        assert 'GAIN_MAP_IFU' in description
        assert '{detector}' not in description

    def test_unpinned_tags_stay_as_placeholders(self):
        """ metis_det_dark serves any detector: only the data can resolve it. """
        description = MetisDetDark._build_description()
        assert 'GAIN_MAP_{detector}' in description


class TestListDataitemsInput:
    def test_finds_inputs_declared_without_a_nested_class(self):
        """ metis_ifu_wavecal binds the module-level DistortionTableInput directly. """
        assert MetisIfuWavecal in set(Recipe._list_dataitems_input(DistortionTable))

    def test_finds_inputs_declared_as_nested_classes(self):
        """ metis_ifu_wavecal binds master_dark to its own nested optional class. """
        assert MetisIfuWavecal in set(Recipe._list_dataitems_input(MasterDark))
