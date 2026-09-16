"""
Tests for static specialization of parametrizable items and containers: it must
resolve names without mutating anything or touching the registries.
"""
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (registers all recipes)
from pymetis.engine.dataitems.dataitem import DataItem
from pymetis.engine.qc.parameter import QcParameter
from pymetis.engine.inputs import PipelineInputSet
from pymetis.engine.recipes import Recipe
from pymetis.instruments.metis.dataitems.img.raw import ImageRaw, LmImageRaw, LmImageSciRaw
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark, MasterDark2rg
from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl
from pymetis.instruments.metis.recipes.lm_lss.metis_lm_lss_rsrf import MetisLmLssRsrfImpl
from pymetis.instruments.metis.recipes.prefab.lss.rsrf import MetisLssRsrfImpl


class TestItemSpecialization:
    def test_resolving_to_a_handwritten_class_returns_it(self):
        assert MasterDark.specialized(detector='2RG') is MasterDark2rg

    def test_parameters_resolving_nothing_return_the_class_itself(self):
        assert MasterDark2rg.specialized(band='LM') is MasterDark2rg

    def test_a_clone_keeps_the_abstractness_of_its_template(self):
        """ There is no MasterDarkNope, so a clone is made; MasterDark is a template. """
        clone = MasterDark.specialized(detector='NOPE')
        assert clone.name() == 'MASTER_DARK_NOPE'
        assert clone._abstract
        assert clone._specialized_from is MasterDark

    def test_an_abstract_clone_is_not_registered(self):
        """ A template without a leaf for the tag is a gap in the catalogue, not an
        entry the specialization may invent. """
        MasterDark.specialized(detector='NOPE')
        assert 'MASTER_DARK_NOPE' not in DataItem._registry

    def test_a_concrete_resolved_clone_is_registered(self):
        """ QC {band} ... templates have no per-band leaves; the resolved clones are
        legitimate catalogue entries and must be findable by tag. """
        clone = MetisLmLssRsrfImpl.Qc.MeanLevel
        assert clone._specialized_from is MetisLssRsrfImpl.Qc.MeanLevel
        assert QcParameter.find('QC LM LSS RSRF MEAN LEVEL') is clone

    def test_parameters_resolving_nothing_return_the_template_itself(self):
        assert MasterDark.specialized(band='LM') is MasterDark   # MASTER_DARK_{detector} has no {band}

    def test_a_partially_resolved_clone_is_not_registered(self):
        """ XX_IMAGE_{target}_RAW is neither a tag nor a hand-written template. """
        clone = ImageRaw.specialized(band='XX')
        assert clone._specialized_from is ImageRaw
        assert clone.name() == 'XX_IMAGE_{target}_RAW'
        assert 'XX_IMAGE_{target}_RAW' not in DataItem._templates
        assert DataItem.find_template('XX_IMAGE_{target}_RAW') is None

    def test_only_relevant_keywords_reach_a_clone(self):
        """ An index meant for a sibling (`order` of LCOEFF{order}) must not make the
        specialization of the whole QC set fail on the items that lack it. """
        from pymetis.instruments.metis.recipes.prefab.lss.trace import MetisLssTraceImpl
        specialized = MetisLssTraceImpl.Qc.specialized(band='XX', order=2)
        assert specialized.InterorderLevel.name() == 'QC XX LSS TRACE INTORDR LEVEL'
        assert specialized.LCoeff.name() == 'QC XX LSS TRACE LCOEFF2'

    def test_a_handwritten_partial_specialization_is_found_by_template(self):
        """ LmImageRaw is abstract (LM_IMAGE_{target}_RAW still has a placeholder), yet
        it must be preferred over a synthesized LM clone of ImageRaw. """
        assert ImageRaw.specialized(band='LM') is LmImageRaw
        assert LmImageRaw._abstract
        assert DataItem.find_template('LM_IMAGE_{target}_RAW') is LmImageRaw

    def test_a_partial_specialization_resolves_further_to_the_leaf(self):
        assert LmImageRaw.specialized(target='SCI') is LmImageSciRaw
        assert ImageRaw.specialized(band='LM', target='SCI') is LmImageSciRaw


class TestRegistryHygiene:
    """ Importing every recipe specializes every container; only resolved tags may land. """

    def test_no_registry_key_carries_placeholders(self):
        for registry in (DataItem._registry, QcParameter._registry):
            assert not [k for k in registry if '{' in k]

    def test_every_registered_class_is_concrete(self):
        for registry in (DataItem._registry, QcParameter._registry):
            assert not [k for k, v in registry.items() if v._abstract]

    def test_templates_hold_only_placeholder_names(self):
        for root in (DataItem, QcParameter):
            assert root._templates
            assert all('{' in k for k in root._templates)

    def test_qc_parameter_base_does_not_register_itself(self):
        """ The template base is abstract; its placeholder name must not own a key. """
        assert "none" not in QcParameter._registry

    def test_ifu_rsrf_publishes_its_own_nbadpix(self):
        """ The DRLD lists QC IFU RSRF NBADPIX; the REDUCE parameter was reused before. """
        from pymetis.instruments.metis.recipes.ifu.metis_ifu_rsrf import MetisIfuRsrf
        description = MetisIfuRsrf._build_description()
        assert 'QC IFU RSRF NBADPIX' in description
        assert 'QC IFU REDUCE NBADPIX' not in description

    def test_partial_product_templates_are_not_registered_by_specialization(self):
        assert 'MASTER_DARK_{detector}' not in DataItem._registry
        assert DataItem.find_template('MASTER_DARK_{detector}') is MasterDark

    def test_a_partially_specialized_data_item_cannot_be_instantiated(self):
        with pytest.raises(TypeError, match='LM_IMAGE_{target}_RAW'):
            LmImageRaw(None)


class TestContainerSpecialization:
    def test_the_declared_container_is_not_mutated(self):
        """ The Impl holds a specialized subclass; the class in its body is untouched. """
        specialized = MetisDetDarkImpl.ProductSet
        declared = specialized._specialized_from
        assert specialized is not declared
        assert specialized._specialized_for is MetisDetDarkImpl
        assert declared.MasterDark is MasterDark

    def test_an_abstract_template_stays_uninstantiable_after_specialization(self):
        """ metis_det_dark pins no detector: its master dark keeps the placeholder. """
        product = MetisDetDarkImpl.ProductSet.MasterDark
        assert product._abstract
        with pytest.raises(TypeError):
            product(None)

    def test_specializing_twice_starts_from_the_original(self):
        twice = MetisDetDarkImpl.ProductSet.specialized(detector='2RG')
        assert twice._specialized_from is MetisDetDarkImpl.ProductSet._specialized_from
        assert twice.MasterDark is MasterDark2rg

    def test_a_subclass_of_a_specialized_container_keeps_its_own_members(self):
        """ `class ProductSet(OtherImpl.ProductSet): Extra = ...` is a hand-written
        container in its own right, not a specialization to start over from. """
        from pymetis.instruments.metis.dataitems.common import PersistenceMap

        class Extended(MetisDetDarkImpl.ProductSet):
            Extra = PersistenceMap

        members = dict(Extended.specialized(detector='2RG').list_classes())
        assert members['Extra'] is PersistenceMap
        assert members['MasterDark'] is MasterDark2rg

    def test_the_recipe_input_set_is_narrowed_to_its_own_tags(self):
        """ A 2RG recipe's master dark input accepts MASTER_DARK_2RG only; the prefab it
        inherits from still declares the generic template for its other children. """
        from pymetis.instruments.metis.recipes.lm_img.metis_lm_img_flat import MetisLmImgFlatImpl
        from pymetis.instruments.metis.recipes.prefab.img.flat import MetisBaseImgFlatImpl
        inputs = dict(MetisLmImgFlatImpl.InputSet.list_input_classes())
        assert inputs['master_dark'].Item is MasterDark2rg
        assert MetisLmImgFlatImpl.InputSet._specialized_for is MetisLmImgFlatImpl
        assert dict(MetisBaseImgFlatImpl.InputSet.list_input_classes())['master_dark'].Item is MasterDark

    def test_an_input_set_specialized_twice_starts_from_the_original(self):
        from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl as Impl
        once = Impl.InputSet.specialized(detector='2RG')
        twice = once.specialized(detector='GEO')
        assert once is not Impl.InputSet
        assert twice._specialized_from is Impl.InputSet


class TestPromotion:
    def test_qc_parameters_without_a_handwritten_leaf_promote_to_the_specialized_clone(self):
        """ QC {band} LSS RSRF MEAN LEVEL has no per-band class; the clone must serve. """
        promoted = MetisLmLssRsrfImpl.Qc.promoted()
        assert promoted.MeanLevel.name() == 'QC LM LSS RSRF MEAN LEVEL'
        assert promoted.MeanLevel(1.0).value == 1.0

    def test_promotion_does_not_mutate_the_container(self):
        before = MetisLmLssRsrfImpl.Qc
        MetisLmLssRsrfImpl.Qc.promoted()
        assert MetisLmLssRsrfImpl.Qc is before

    def test_an_abstract_template_without_a_leaf_raises(self):
        with pytest.raises(TypeError, match='MASTER_DARK_NOPE'):
            MetisDetDarkImpl.ProductSet.promoted(detector='NOPE')


class TestIndexedQcParameters:
    """ `LCOEFF{order}`-style QC names carry an index the recipe fills per value; the
    index is not a data tag and must survive promotion (metis_lm_lss_trace failed in
    EDPS when promotion treated it as an unresolved tag). """

    def test_promotion_keeps_index_placeholders(self):
        from pymetis.instruments.metis.recipes.lm_lss.metis_lm_lss_trace import MetisLmLssTraceImpl
        promoted = MetisLmLssTraceImpl.Qc.promoted()
        assert promoted.LCoeff.name() == 'QC LM LSS TRACE LCOEFF{order}'
        assert not promoted.LCoeff._abstract

    def test_an_unfilled_index_cannot_be_emitted(self):
        """ A QC value with `{order}` in its name would become an invalid FITS card. """
        from pymetis.instruments.metis.recipes.lm_lss.metis_lm_lss_trace import MetisLmLssTraceImpl
        with pytest.raises(TypeError, match='still has placeholders'):
            MetisLmLssTraceImpl.Qc.promoted().LCoeff(1.0)

    def test_an_index_is_filled_per_value(self):
        """ `order` is not a tag keyword, but it is a placeholder of this very name, so
        specialization accepts it; how many orders there will be is only known at run time. """
        from pymetis.instruments.metis.recipes.lm_lss.metis_lm_lss_trace import MetisLmLssTraceImpl
        coefficient = MetisLmLssTraceImpl.Qc.LCoeff.specialized(order=2)
        assert coefficient.name() == 'QC LM LSS TRACE LCOEFF2'
        assert coefficient(1.0).value == 1.0

    def test_a_typo_is_still_refused(self):
        with pytest.raises(TypeError, match="unknown tag parameter"):
            class Typo(MasterDark, bnad='LM'):   # noqa: F841
                pass

    def test_promotion_still_requires_every_tag(self):
        from pymetis.instruments.metis.recipes.prefab.lss.trace import MetisLssTraceImpl
        with pytest.raises(TypeError, match=r"tags \['band'\] remain unresolved"):
            MetisLssTraceImpl.Qc.promoted()


DATA_TAG_DEFAULTS = dict(band='LM', detector='2RG', target='SCI', source='LAMP', cgrph='RAVC')


@pytest.mark.parametrize('recipe', sorted(Recipe._registry.values(), key=lambda r: r._name),
                         ids=lambda r: r._name)
def test_every_recipe_input_resolves_to_a_handwritten_item(recipe):
    """ Narrowing an input must land on a catalogue class (leaf or hand-written partial
    template), never on a synthesized clone, which would match no frame at all. """
    for name, input_class in recipe.Impl.InputSet.list_input_classes():
        assert not hasattr(input_class.Item, '_specialized_from'), \
            f"{recipe._name}.{name}: {input_class.Item.name()} has no hand-written class"


@pytest.mark.parametrize('recipe', sorted(Recipe._registry.values(), key=lambda r: r._name),
                         ids=lambda r: r._name)
def test_every_recipe_promotes_its_qc_set_once_the_data_tags_are_known(recipe):
    """ What RecipeImpl.__init__ does at run time, with a stand-in for the tags the
    frames would supply; the recipe's own tags take precedence. """
    recipe.Impl.Qc.promoted(**(DATA_TAG_DEFAULTS | recipe.Impl.tag_parameters()))


@pytest.mark.parametrize('recipe', sorted(Recipe._registry.values(), key=lambda r: r._name),
                         ids=lambda r: r._name)
def test_every_recipe_has_a_raw_role_input(recipe):
    """ CPL DFS inherits the product header from the first RAW frame; without one it falls
    back to whichever calibration comes first in the SOF. """
    import cpl
    assert any(input_class._group == cpl.ui.Frame.FrameGroup.RAW
               for _, input_class in recipe.Impl.InputSet.list_input_classes()), \
        f"{recipe._name}: no input is marked with PrimaryInputMixin or derived from RawInput"


class TestInputSetSpecializationGuards:
    def test_templates_are_abstract(self):
        """ The README promises it; a concrete class with a placeholder in its name would
        be instantiable with an unresolved tag. """
        assert all(template._abstract for template in DataItem._templates.values())

    def test_an_input_never_binds_a_clone(self):
        """ A clone is a sibling of the leaves: no frame could ever match it. """
        from pymetis.instruments.metis.inputs.common import MasterDarkInput

        class Probe(PipelineInputSet):
            master_dark: MasterDarkInput

        with pytest.raises(TypeError, match='no hand-written class'):
            Probe.specialized(detector='NOPE')
