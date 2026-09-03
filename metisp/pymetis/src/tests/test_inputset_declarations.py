"""
Tests for the explicit, annotation-based declaration of InputSet attributes:
the per-defining-class verification (enforced at class creation), annotation-based
recipe introspection, and the specialize/promote machinery on input sets.
"""
import sys
import textwrap
import types

import pytest

from pymetis.engine.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.engine.recipes.recipe import Recipe

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the recipe registry)
from pymetis.instruments.metis.dataitems.distortion.table import DistortionTable
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark, MasterDarkIfu
from pymetis.instruments.metis.inputs.common import MasterDarkInput
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
    """ The verifier runs from `__init_subclass__`, so class creation is the test. """

    def test_accepts_a_nested_class_bound_in_its_own_body(self):
        class InputSet(PipelineInputSet):
            class NestedInput(AlphaInput):
                pass

            alpha: NestedInput

    def test_accepts_an_override_by_reannotation_alone(self):
        """ The parent's nested class stays behind, bound at the parent's level. """
        class ChildInputSet(ParentInputSet):
            alpha: BetaInput

        assert dict(ChildInputSet.list_input_classes()) == {'alpha': BetaInput}

    def test_rejects_a_nested_class_left_unannotated(self):
        """ Overriding the nested class without re-annotating would silently
        instantiate the inherited annotation's class instead: the class
        definition itself must fail, at import time. """
        with pytest.raises(TypeError, match="NestedInput"):
            class ChildInputSet(ParentInputSet):
                class NestedInput(BetaInput):
                    pass


class TestStringifiedAnnotations:
    def test_inputs_survive_a_future_annotations_import(self):
        """ With `from __future__ import annotations` (and under PEP 649 lazy
        evaluation), annotations arrive as strings; they must still resolve,
        including ones naming a nested input class. """
        source = textwrap.dedent("""
            from __future__ import annotations

            from pymetis.engine.inputs import PipelineInputSet
            from pymetis.instruments.metis.inputs.common import MasterDarkInput, GainMapInput

            class FutureInputSet(PipelineInputSet):
                class NestedGainMapInput(GainMapInput):
                    pass

                master_dark: MasterDarkInput
                gain_map: NestedGainMapInput
        """)
        module = types.ModuleType('pymetis_test_future_annotations')
        sys.modules[module.__name__] = module
        try:
            exec(compile(source, '<test>', 'exec'), module.__dict__)
            found = dict(module.FutureInputSet.list_input_classes())
            assert found['master_dark'] is MasterDarkInput
            assert found['gain_map'] is module.FutureInputSet.NestedGainMapInput
        finally:
            del sys.modules[module.__name__]


class TestSpecializeAndPromote:
    """
    `specialize` and `promoted` rebind each input's `Item` through the class's own
    annotations -- the machinery mirrors ProductSet's, adapted to inputs.
    """

    def test_specialize_rebinds_the_item_to_the_registered_class(self):
        class InputSet(PipelineInputSet):
            master_dark: MasterDarkInput

        InputSet.specialize(detector='IFU')

        specialized = dict(InputSet.list_input_classes())['master_dark']
        assert specialized.Item is MasterDarkIfu
        assert issubclass(specialized, MasterDarkInput)
        # the shared module-level input class is untouched
        assert MasterDarkInput.Item is MasterDark

    def test_specialize_without_matching_parameters_changes_nothing(self):
        class InputSet(PipelineInputSet):
            master_dark: MasterDarkInput

        InputSet.specialize(target='SCI')

        assert dict(InputSet.list_input_classes())['master_dark'] is MasterDarkInput

    def test_promoted_returns_a_subclass_and_does_not_mutate(self):
        class InputSet(PipelineInputSet):
            master_dark: MasterDarkInput

        promoted = InputSet.promoted(detector='IFU')

        assert promoted is not InputSet
        assert issubclass(promoted, InputSet)
        assert dict(promoted.list_input_classes())['master_dark'].Item is MasterDarkIfu
        assert dict(InputSet.list_input_classes())['master_dark'] is MasterDarkInput

    def test_promoted_raises_on_an_unregistered_tag(self):
        class InputSet(PipelineInputSet):
            master_dark: MasterDarkInput

        with pytest.raises(TypeError, match="MASTER_DARK_NOPE"):
            InputSet.promoted(detector='NOPE')


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
