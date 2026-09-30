"""
The DPR classification of the raw data items against the EDPS classification rules.

Every RAW-group item declares its (DPR.CATG, DPR.TECH, DPR.TYPE) triple (`DataItem._dpr`);
`metis.metis_classification` must assign the item's tag to exactly that triple, and every rule
that classifies raw data must name a catalogue item. The rule-based tests skip without edps or
the workflow package; the declaration tests always run. Disagreements are strict xfails, listed
per test, so that a fix flips exactly one entry to an unexpected pass.
"""
import cpl
import pytest

import pymetis.instruments.metis.dataitems  # noqa: F401  (fills the item registry)
from pymetis.engine.dataitems import DataItem

RAW_ITEMS: dict[str, type[DataItem]] = {
    tag: item for tag, item in sorted(DataItem._registry.items())
    if '{' not in tag and item.frame_group() == cpl.ui.Frame.FrameGroup.RAW
}


def _rules():
    try:
        from pymetis.engine import workflows as wf
        return wf.classification_rules(), wf
    except (ImportError, FileNotFoundError):
        return None, None


RULES, wf = _rules()
DPR_RULES = {tag: rule for tag, rule in (RULES or {}).items() if any(k in rule for k in wf.DPR_KEYWORDS)} if RULES else {}


def expect_failure(request, key: str, names: frozenset[str], reason: str) -> None:
    if key in names:
        request.applymarker(pytest.mark.xfail(strict=True, reason=reason))


# Snapshot of the disagreements on 2026-09-29; remove an entry once the rule or the item is fixed.
RAW_ITEMS_WITHOUT_A_RULE = frozenset({          # raw items metis_classification.py does not know
    'LM_CHOPHOME_RAW', 'LM_PUPIL_RAW', 'N_PUPIL_RAW', 'N_OFF_AXIS_PSF_RAW',
})
RULES_DISAGREEING_WITH_THE_ITEM = frozenset({   # the rule's triple differs from the item's
    'LM_IMAGE_SKY_RAW',                         # the rule leaves DPR.CATG free (CALIB or SCIENCE); the item says CALIB
})
DPR_RULES_WITHOUT_AN_ITEM = frozenset({         # rules classifying raw data no catalogue item carries
    'LM_LSS_SKY_RAW', 'N_IMAGE_SKY_RAW', 'N_LSS_SKY_RAW',
})
PRODUCT_RULES_WITHOUT_AN_ITEM = frozenset()


class TestDprDeclaration:
    @pytest.fixture(params=sorted(RAW_ITEMS), ids=lambda tag: tag)
    def tag(self, request) -> str:
        return request.param

    def test_every_raw_item_declares_a_dpr_triple(self, tag):
        assert RAW_ITEMS[tag].dpr() is not None, f"{tag} declares no _dpr"

    def test_the_triple_is_fully_resolved(self, tag):
        dpr = RAW_ITEMS[tag].dpr()
        assert dpr is not None and all(part and '{' not in part for part in dpr), f"{tag}: {dpr}"

    def test_only_raw_items_declare_a_triple(self):
        wrong = [tag for tag, item in DataItem._registry.items()
                 if '{' not in tag and item.frame_group() != cpl.ui.Frame.FrameGroup.RAW and item.dpr() is not None]
        assert wrong == []


@pytest.mark.skipif(RULES is None, reason="edps or the metis workflow package is not available")
class TestClassificationRules:
    @pytest.fixture(params=sorted(RAW_ITEMS), ids=lambda tag: tag)
    def tag(self, request) -> str:
        return request.param

    def test_every_raw_item_has_a_rule(self, tag, request):
        expect_failure(request, tag, RAW_ITEMS_WITHOUT_A_RULE, "no classification rule for this tag")
        assert tag in RULES, f"no classification rule assigns {tag}"

    def test_the_rule_says_what_the_item_declares(self, tag, request):
        if tag not in RULES:
            pytest.skip("no rule")
        expect_failure(request, tag, RULES_DISAGREEING_WITH_THE_ITEM, "the rule's DPR triple differs")
        assert {k.edps: v for k, v in RAW_ITEMS[tag].dpr_rule().items()} == dict(zip(wf.DPR_KEYWORDS, wf.rule_dpr(RULES[tag])))

    @pytest.mark.parametrize('rule_tag', sorted(DPR_RULES), ids=lambda tag: tag)
    def test_every_dpr_rule_names_a_raw_item(self, rule_tag, request):
        expect_failure(request, rule_tag, DPR_RULES_WITHOUT_AN_ITEM, "the rule names no raw item of the catalogue")
        assert rule_tag in RAW_ITEMS, f"rule {rule_tag} classifies raw data but no raw item carries that tag"

    @pytest.mark.parametrize('rule_tag', sorted(set(RULES or {}) - set(DPR_RULES)), ids=lambda tag: tag)
    def test_every_product_rule_names_a_catalogue_item(self, rule_tag, request):
        expect_failure(request, rule_tag, PRODUCT_RULES_WITHOUT_AN_ITEM, "the rule names no catalogue item")
        assert DataItem.find(rule_tag) is not None, f"rule {rule_tag} names no catalogue item"
