# METIS LM/N LSS EDPS workflows
This branch contains the first draft of the LSS LM/N-band EDPS workflows. Note that this is work in progress and far from being perfect.

## Included files:
```
README                          - This description
build_*_graphs.sh               - Bash scripts to easier produce workflow graphs
*.png                           - Workflow graphs created by EDPS
metis_*_lss_wkf.py              - Main EDPS workflow scripts
metis_*_lss_datasources.py      - Contains data sources description
metis_*_lss_classification.py   - Contains classification rule description
metis_*_keywords.py             - List of header keywords used by the pipeline/EDPS
metis_*_rules.py                - Rules and functions for association/classification of files
metis_*_task_functions.py       - Auxiliary functions required by tasks
```

more files to follow...

## Changelog:

```
2024-08-08      V0.0.1          First prototype of *wkf.py, *datasources.py, *classification.py
2024-08-14      V0.0.2          Including first comments of Lodo, adding first versions of
                                supplementary files according to EDPS Tutorial (Lodo et al. 2023-12-01)
```
