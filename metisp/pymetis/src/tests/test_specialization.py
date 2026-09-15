"""
Tests for static specialization of parametrizable items and containers: it must
resolve names without mutating anything or touching the registries.
"""
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (registers all recipes)
from pymetis.engine.dataitems.dataitem import DataItem
from pymetis.engine.qc.parameter import QcParameter
from pymetis.instruments.metis.dataitems.img.raw import ImageRaw, LmImageRaw, LmImageSciRaw
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark, MasterDark2rg
from pymetis.instruments.metis.recipes.metis_det_dark import MetisDetDarkImpl
from pymetis.instruments.metis.recipes.lm_lss.metis_lm_lss_rsrf import MetisLmLssRsrfImpl


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
        assert clone._specialized_from is not None
        assert QcParameter.find('QC LM LSS RSRF MEAN LEVEL') is clone

    def test_a_partially_resolved_clone_is_not_registered(self):
        clone = MasterDark.specialized(band='LM')   # resolves nothing in MASTER_DARK_{detector}
        assert clone is MasterDark


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
            assert all('{' in k for k in root._templates)

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
