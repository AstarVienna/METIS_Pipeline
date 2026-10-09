# DRLD regenerated from the pipeline

This is the METIS Data Reduction Library Design document (E-REP-AST-MET-1006) with every part
that the pipeline can generate replaced by a generated fragment: data-item cards, recipe cards,
QC-parameter cards, per-recipe flowcharts, the association maps, the DPR table and the
matched-keywords table are `\input` from `generated/`, written by `metisp/drld/generate_drld.py`
from the `pymetis` catalogue and the EDPS workflows. The prose is the approved document's.

Origin: the working tree of the DRLD repository (`astar/drld`, commit 2156d1e plus local edits)
as copied on 2026-10-09 and retrofitted by `metisp/drld/retrofit.py`. `generated/RETROFIT.md`
lists what was replaced, what the pipeline does not define and therefore stays hand-written
(orange boxes in the PDF, `% HANDWRITTEN` in the source), and what the pipeline adds.

Build: `make METIS_DRLD.pdf` (pdflatex, biber; about ten minutes). After a change to the pipeline,
`make regenerate` re-emits the fragments from the development venv; the chapter files are not
touched again. The approved document lives in its own repository; this tree is experimental.
