"""
The keyword vocabulary against its users, both ways: every keyword an item, a recipe or a
header read refers to is a registered vocabulary keyword, and every vocabulary keyword is
referred to somewhere or listed as declared-only. No literal header spelling survives in the
instrument code: the vocabulary is the one place a keyword is spelled.
"""
import re
from pathlib import Path

import cpl
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the registries)
from pymetis.engine.dataitems import DataItem
from pymetis.engine.keywords import Keyword, Alias
from pymetis.engine.keywords.usage import keywords_read_in
from pymetis.engine.recipes import Recipe
from pymetis.instruments.metis import keywords as kw

ITEMS = {tag: item for tag, item in sorted(DataItem._registry.items()) if '{' not in tag}
# The vocabulary proper: the registry also holds whatever other tests declared in this session.
VOCABULARY = {k for k in vars(kw).values() if isinstance(k, Keyword)}
RECIPES = dict(sorted(Recipe._registry.items()))
INSTRUMENT = Path(kw.__file__).parent


def expect_failure(request, key: str, names: frozenset[str], reason: str) -> None:
    if key in names:
        request.applymarker(pytest.mark.xfail(strict=True, reason=reason))


# Snapshot of the disagreements on 2026-09-30; remove an entry once the recipe or the item is fixed.
MATCHED_NOT_ON_A_RAW_INPUT = frozenset({
    'metis_img_adi_cgrph',                              # matches on DRS.MASK; the calibrated frames declare DRS.FILTER
    'metis_lm_img_distortion', 'metis_n_img_distortion',  # match on DRS.FILTER; the distortion raws declare DRS.IFU (for LM and N too)
    'metis_lm_adc_slitloss', 'metis_n_adc_slitloss',    # match on DRS.SLIT; the slit-loss raws declare PRO.CATG only
    'metis_lm_lss_wave',                                # matches on DRS.SLIT; the wave raws declare the DPR triple only
    'metis_lm_lss_mf_calctrans', 'metis_n_lss_mf_calctrans',   # match on DRS.SLIT; their raw input is the molecfit best-fit table
})
# On every raw frame by FITS convention, so no card lists them as OCA keywords; matching on them is always possible.
EXPOSURE_KEYWORDS = frozenset({kw.DET_DIT, kw.DET_NDIT})


class TestDeclarations:
    @pytest.fixture(params=sorted(ITEMS), ids=lambda tag: tag)
    def item(self, request) -> type[DataItem]:
        return ITEMS[request.param]

    @pytest.fixture(params=sorted(RECIPES), ids=lambda name: name)
    def recipe(self, request) -> type[Recipe]:
        return RECIPES[request.param]

    def test_oca_keywords_are_vocabulary_keywords(self, item):
        for keyword in item.oca_keywords():
            assert isinstance(keyword, Keyword) and Keyword.registry.get(keyword.name) == keyword, \
                f"{item.name()}: {keyword!r} is not a vocabulary keyword"

    def test_matched_keywords_are_vocabulary_keywords(self, recipe):
        for keyword in recipe._matched_keywords or ():
            assert isinstance(keyword, Keyword) and Keyword.registry.get(keyword.name) == keyword, \
                f"{recipe._name}: {keyword!r} is not a vocabulary keyword"

    def test_matched_keywords_are_carried_by_a_raw_input(self, recipe, request):
        """ A recipe matches calibrations to its raw data on keywords the raw data carries (its OCA keywords). """
        expect_failure(request, recipe._name, MATCHED_NOT_ON_A_RAW_INPUT, "matched keyword not among the raw inputs' OCA keywords")
        carried: set[Keyword] = set()
        for _, inp in recipe._list_inputs():
            if inp._group == cpl.ui.Frame.FrameGroup.RAW:
                carried |= set(inp.Item.oca_keywords())
        missing = set(recipe._matched_keywords or ()) - carried - EXPOSURE_KEYWORDS
        assert not missing, f"{recipe._name} matches on {sorted(k.name for k in missing)}, which no raw input declares"


def used_keywords() -> dict[Keyword, set[str]]:
    """ Every vocabulary keyword something refers to, and by what. """
    used: dict[Keyword, set[str]] = {}
    for tag, item in ITEMS.items():
        for keyword in item.oca_keywords():
            used.setdefault(keyword, set()).add(f"oca:{tag}")
    for name, recipe in RECIPES.items():
        for keyword in recipe._matched_keywords or ():
            used.setdefault(keyword, set()).add(f"matched:{name}")
        for keyword in keywords_read_in(recipe, kw):
            used.setdefault(keyword, set()).add(f"read:{name}")
    for keyword in list(used):
        if isinstance(keyword, Alias):
            for target in keyword.resolves_to:
                used.setdefault(target, set()).add(f"alias:{keyword.name}")
    for keyword in DataItem.DPR + (kw.PRO_CATG,):
        used.setdefault(keyword, set()).add("engine")
    try:                                                    # the EDPS workflows group, match and classify on keywords too
        from pymetis.engine import workflows as wf
        if wf.workflows_directory() is not None:
            for spelling, where in wf.keyword_usage(('metis.metis_wkf', 'metis.metis_lm_app_wkf', 'metis.metis_lm_ravc_wkf')).items():
                keyword = Keyword.from_edps(spelling)
                if keyword is not None:
                    used.setdefault(keyword.template or keyword, set()).update(where)
    except ImportError:
        pass
    return used


class TestVocabulary:
    def test_every_keyword_is_used_or_declared_only(self):
        used = used_keywords()
        idle = {k for k in VOCABULARY if k not in used} - kw.DECLARED_ONLY
        assert not idle, f"vocabulary keywords nothing refers to and not declared-only: {sorted(k.name for k in idle)}"

    def test_declared_only_keywords_are_really_unused(self):
        used = used_keywords()
        busy = {k: used[k] for k in kw.DECLARED_ONLY if k in used}
        assert not busy, f"declared-only keywords that are in use: { {k.name: sorted(v) for k, v in busy.items()} }"

    def test_aliases_resolve_to_vocabulary_keywords(self):
        for alias in (k for k in VOCABULARY if isinstance(k, Alias)):
            for target in alias.resolves_to:
                assert Keyword.registry.get(target.name) == target, f"{alias.name} resolves to unknown {target.name}"

    def test_no_literal_header_spelling_in_the_instrument_code(self):
        """ 'ESO DET DIT' and header['...'] belong to the vocabulary, not to recipes. """
        literal = re.compile(r"""["']ESO [A-Z]|f["']ESO |header\w*\[\s*f?["']""")
        hits = [f"{path.relative_to(INSTRUMENT)}:{n}: {line.strip()}"
                for path in sorted(INSTRUMENT.rglob('*.py')) if 'tests' not in path.parts
                for n, line in enumerate(path.read_text().splitlines(), 1)
                if literal.search(line) and not line.lstrip().startswith('#')]
        assert hits == [], "literal header keywords:\n" + "\n".join(hits)
