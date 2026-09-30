"""
The EDPS workflow package's keyword strings against the vocabulary. The package stays
independent of pymetis (EDPS imports it in its own environment), so its lowercase strings
are checked here: every grouping, matching and classification keyword, every constant and
every header read is the EDPS spelling of a registered keyword, and no workflow reads an alias,
which EDPS cannot resolve. Strings that are no METIS keyword at all are a strict xfail list.
"""
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401
from pymetis.engine.keywords import Keyword, Alias
from pymetis.instruments.metis import keywords as kw

pytest.importorskip('edps')
from pymetis.engine import workflows as wf  # noqa: E402

if wf.workflows_directory() is None:
    pytest.skip("the metis workflow package is not available (set $METIS_WORKFLOWS)", allow_module_level=True)

WORKFLOWS = ('metis.metis_wkf', 'metis.metis_lm_app_wkf', 'metis.metis_lm_ravc_wkf')
USAGE = wf.keyword_usage(WORKFLOWS)

# Snapshot of 2026-09-30: strings of the workflow package that are no METIS keyword. Drop them there, then here.
NOT_A_KEYWORD = frozenset({
    'simple',                                   # a placeholder match keyword in metis_datasources.py
    'filt.id', 'ocs.enabled.fe', 'ins5.modsel.id', 'det.binx', 'det.biny', 'telescop', 'date',
    'obs.targ.name', 'tel.airm.start',          # constants copied from another instrument's workflow, never used
})


@pytest.mark.parametrize('spelling', sorted(USAGE), ids=lambda s: s)
def test_every_workflow_keyword_is_in_the_vocabulary(spelling, request):
    if spelling in NOT_A_KEYWORD:
        request.applymarker(pytest.mark.xfail(strict=True, reason="not a METIS keyword"))
    keyword = Keyword.from_edps(spelling)
    assert keyword is not None, f"{spelling!r} ({sorted(USAGE[spelling])}) is no vocabulary keyword"
    assert not isinstance(keyword, Alias), f"{spelling!r} is an alias; EDPS cannot resolve it, read {keyword.edps_alternatives}"


def test_the_filter_alternatives_match_the_alias():
    import importlib
    constants = importlib.import_module('metis.metis_keywords')
    assert tuple(constants.drs_filter_alternatives) == kw.DRS_FILTER.edps_alternatives


def test_the_dpr_rules_use_the_dpr_keywords():
    rule_keys = {s for s, where in USAGE.items() if any(w.startswith('rule:') for w in where)}
    assert rule_keys >= set(wf.DPR_KEYWORDS)
    assert rule_keys <= set(wf.DPR_KEYWORDS) | {kw.INSTRUME.edps, kw.PRO_CATG.edps}
