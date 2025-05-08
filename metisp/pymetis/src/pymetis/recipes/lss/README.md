# METIS LM LSS Pipeline
This is a first draft of the LM LSS skeleton for the METIS instrument

## Added files

- pyesorex skeleton recipes (`METIS_Pipeline/metisp/pymetis/src/pymetis/recipes/lss`):
```
metis_lm_lss_rsrf.py
metis_lm_lss_trace.py
metis_lm_lss_wave.py
metis_lm_lss_std.py
metis_lm_lss_sci.py
metis_lm_lss_mf_model.py
metis_lm_lss_mf_calctrans.py
metis_lm_lss_mf_correct.py
metis_lm_lss_test.py  (for testing purposes only)
metis_lss_utils.py (remnant, not needed for the time being)
```

- Test scripts (`METIS_Pipeline/metisp/pymetis/src/pymetis/tests/recipes/lss`):

```
test_metis_lm_lss_mf_calctrans.py
test_metis_lm_lss_mf_correct.py
test_metis_lm_lss_mf_model.py
test_metis_lm_lss_rsrf.py
test_metis_lm_lss_sci.py
test_metis_lm_lss_std.py
test_metis_lm_lss_trace.py
test_metis_lm_lss_wave.py

```

- EDPS workflows (`/daten/ELT/METIS/Vienna_git/MAIN_branch/METIS_Pipeline/metisp/workflows/metis`):
CAVEAT: NOT YET fully functional!
```
metis_lm_lss_classification.py
metis_lm_lss_datasources.py
metis_lm_lss_keywords.py
metis_lm_lss_molecfit.py
metis_lm_lss_rules.py
metis_lm_lss_task_functions.py
metis_lm_lss_wkf.py
```

## Other modified files
```
METIS_Pipeline/metisp/pymetis/src/pymetis/classes/inputs/__init__.py
METIS_Pipeline/metisp/pymetis/src/pymetis/classes/inputs/common.py
METIS_Pipeline/metisp/pyrecipes/metis_recipes.py
```

## Remarks
This is a snapshot only! There might be bugs/inconsistencies and other suprises!
EDPS is not yet fully functional!

