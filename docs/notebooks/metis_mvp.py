#!/usr/bin/env python3
"""
metis_mvp.py — Minimal end-to-end METIS workflow script

Workflow:
  1. Simulate LM dark frames via METIS_Simulations (ScopeSim)
  2. Build a CPL frameset from the simulated FITS files
  3. Reduce the raw darks with MetisDetDark to produce a master dark
  4. (Skeleton) Ingest raw frames into the MetisWISE archive

Run from the MTR root directory:
    python metis_mvp.py
"""

# ── Section 0: Environment bootstrap ────────────────────────────────────────
# DEFAULT_IRDB_LOCATION must be set before scopesim is imported.

import os
import sys
from pathlib import Path

MTR_ROOT   = Path(__file__).resolve().parent
SIMS_ROOT  = MTR_ROOT / "METIS_Simulations"
PIPE_ROOT  = MTR_ROOT / "METIS_Pipeline" / "metisp" / "pymetis" / "src"
RECIPE_DIR = MTR_ROOT / "METIS_Pipeline" / "metisp" / "pyrecipes"
INST_PKGS  = MTR_ROOT / "inst_pkgs"

sys.path.insert(0, str(SIMS_ROOT))
sys.path.insert(0, str(PIPE_ROOT))

os.environ.setdefault("DEFAULT_IRDB_LOCATION", str(INST_PKGS))
os.environ.setdefault("PYCPL_RECIPE_DIR",       str(RECIPE_DIR))
os.environ.setdefault("PYESOREX_PLUGIN_DIR",    str(RECIPE_DIR))
os.environ.setdefault("PYESOREX_MSG_LEVEL",     "debug")
os.environ.setdefault("PYESOREX_LOG_LEVEL",     "debug")

import scopesim as sim
sim.rc.__config__["!SIM.file.local_packages_path"] = str(INST_PKGS)

if not (INST_PKGS / "METIS").is_dir():
    print(f"Instrument packages not found at {INST_PKGS}. Downloading …")
    INST_PKGS.mkdir(parents=True, exist_ok=True)
    sim.download_packages("METIS", release="2026-02-18")
    sim.download_packages("ELT",   release="2025-10-26")


# ── Section 1: Simulation ────────────────────────────────────────────────────
# Runs ScopeSim via METIS_Simulations to produce synthetic LM dark FITS frames.
# darkLM.yaml: mode=img_lm, source=empty_sky, DIT=1s, nObs=3

from metis_simulations.runSimulationBlock import runSimulationBlock

YAML_FILE = SIMS_ROOT / "YAML" / "ESO" / "darkLM.yaml"
SIM_OUT   = MTR_ROOT / "output" / "sim"
SIM_OUT.mkdir(parents=True, exist_ok=True)

params = dict(
    outputDir = str(SIM_OUT),
    small     = False,
    doStatic  = False,
    doCalib   = 0,
    sequence  = False,
    startMJD  = None,
    calibFile = None,
    nCores    = 1,
    testRun   = False,
)

print(f"\n=== Step 1: Simulating dark frames → {SIM_OUT} ===")
runSimulationBlock([str(YAML_FILE)], params, [])

dark_files = sorted(SIM_OUT.glob("METIS.DARK_LM_RAW.*.fits"))
print(f"Produced {len(dark_files)} dark frame(s):")
for f in dark_files:
    print(f"  {f.name}")


# ── Section 2: Frameset construction ────────────────────────────────────────
# Build a CPL FrameSet from the simulated files.
# The recipe Dark2rgRaw expects tag DARK_2RG_RAW
# (template DARK_{detector}_RAW with detector='2RG').

from pymetis.engine.core.functions.frameset import from_tags

print("\n=== Step 2: Building frameset ===")
frameset = from_tags(
    DARK_2RG_RAW=[str(f.absolute()) for f in dark_files],
)
print(f"Frameset contains {len(list(frameset))} frame(s) tagged DARK_2RG_RAW")


# ── Section 3: Pipeline reduction ───────────────────────────────────────────
# Run MetisDetDark to combine the raw darks into a master dark.
# The recipe writes output files relative to the current working directory.

from pymetis.instruments.metis.recipes import MetisDetDark

PIPE_OUT = MTR_ROOT / "output" / "pipeline"
PIPE_OUT.mkdir(parents=True, exist_ok=True)
_orig_cwd = Path.cwd()
os.chdir(PIPE_OUT)

print(f"\n=== Step 3: Running MetisDetDark → {PIPE_OUT} ===")
try:
    result_frameset = MetisDetDark().run(frameset, {})
    print("Pipeline products:")
    for frame in result_frameset:
        print(f"  [{frame.tag}]  {frame.filename}")
finally:
    os.chdir(_orig_cwd)


# ── Section 4: Archive ingestion skeleton (MetisWISE) ───────────────────────
# Currently blocked by server connectivity.
# Pattern from: /mnt/d/Repos/METIS_Pipeline/docs/notebooks/ingestraw.ipynb

print("\n=== Step 4: MetisWISE ingestion (skeleton — server not yet reachable) ===")

# from metiswise.main.aweimports import *
# from metiswise.main.raw import DARK_2RG_RAW as Dark2rgRawClass
#
# for fits_file in dark_files:
#     raw_obj = Dark2rgRawClass(str(fits_file))
#     print(f"  {fits_file.name}  DIT={raw_obj.det_dit}  NDIT={raw_obj.det_ndit}")
#     raw_obj.store()    # upload file bytes to bulk storage
#     raw_obj.commit()   # persist metadata to the database
#
# # Query example — filter by integration time
# results = Dark2rgRawClass.det_dit < 5
# print(f"Found {len(results)} archive entries with DIT < 5 s")

print("Ingestion skipped (uncomment Section 4 when server is reachable).")
print("\nDone.")
