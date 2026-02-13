# METIS Pipeline on Windows (WSL)

This guide provides step-by-step instructions for installing and testing the METIS Pipeline
on Windows using WSL (Windows Subsystem for Linux) with Ubuntu 24.04.

## Prerequisites

- Windows 10/11 with WSL2 enabled
- At least 8 GB of free disk space
- The METIS Pipeline repository cloned to a Windows drive (e.g. `D:\Repos\METIS_Pipeline`)
- Simulated FITS data **already extracted** to a known location (e.g. `D:\simData_202602`)
  - The scripts expect an `output/` subdirectory containing: `imgLM/`, `imgN/`, `ifu/`,
    `lssLM/`, `lssN/`, `Calib/`, `hciAppLM/`, `hciRavcLM/`, `hciRavcIfu/`
  - If your data is still in a `.tar` file, extract it first

## Overview

The process has four steps:

1. **Setup** - Create a fresh Ubuntu WSL instance and install all dependencies
2. **EDPS Workflows** - Run EDPS to process simulated data and generate SOF files
3. **Create SOF files** - Map EDPS output to the naming convention expected by tests
4. **Pytest** - Run the test suite

## Quick Start

All scripts accept a `--data-dir` argument to specify your simulated data location.
If not provided, they default to `/mnt/d/simData_202602/output`.

**Important**: Because these scripts live on a Windows filesystem, they have `\r\n` line
endings that bash cannot execute directly. The commands below use `sed` to strip them.

```powershell
# From PowerShell on Windows:

# Step 0: Create fresh WSL Ubuntu (WARNING: --unregister destroys existing instance)
wsl --unregister Ubuntu-24.04
wsl --install Ubuntu-24.04
# Complete the initial Ubuntu user setup (create username/password), then continue below.

# Step 1: Install everything (runs as root via sudo)
wsl -d Ubuntu-24.04 -- bash -c "sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/01_setup_wsl.sh | sudo bash"

# Step 2: Run all EDPS workflows (~15 minutes)
# Replace the --data-dir path with your actual data location
wsl -d Ubuntu-24.04 -- bash -c "sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/02_run_edps_workflows.sh | sudo bash -s -- --data-dir /mnt/d/simData_202602/output"

# Step 3: Create SOF files for tests
wsl -d Ubuntu-24.04 -- bash -c "sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/03_create_sof_files.sh | sudo bash"

# Step 4: Run the test suite
wsl -d Ubuntu-24.04 -- bash -c "sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/04_run_tests.sh | sudo bash -s -- --data-dir /mnt/d/simData_202602/output"
```

**Alternatively**, if you have `dos2unix` or are working from inside WSL directly:

```bash
# Inside WSL (as root):
SCRIPTS=/mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl

# Step 1
sed 's/\r$//' $SCRIPTS/01_setup_wsl.sh | bash

# Step 2
sed 's/\r$//' $SCRIPTS/02_run_edps_workflows.sh | bash -s -- --data-dir /mnt/d/simData_202602/output

# Step 3
sed 's/\r$//' $SCRIPTS/03_create_sof_files.sh | bash

# Step 4
sed 's/\r$//' $SCRIPTS/04_run_tests.sh | bash -s -- --data-dir /mnt/d/simData_202602/output
```

## Detailed Guide

See [INSTALL.md](INSTALL.md) for the full walkthrough with explanations.

## Files in this directory

| File | Description |
|------|-------------|
| [README.md](README.md) | This file |
| [INSTALL.md](INSTALL.md) | Detailed installation and testing guide |
| [01_setup_wsl.sh](01_setup_wsl.sh) | Install system packages, create venv, install Python packages |
| [02_run_edps_workflows.sh](02_run_edps_workflows.sh) | Run all EDPS workflows and accumulate output |
| [03_create_sof_files.sh](03_create_sof_files.sh) | Map EDPS output to test-expected SOF naming |
| [04_run_tests.sh](04_run_tests.sh) | Run the pytest test suite |

## Configurable Paths

All scripts have configurable variables at the top. The most important ones:

| Variable | Default | Where |
|----------|---------|-------|
| `REPO_PATH` | `/mnt/d/Repos/METIS_Pipeline` | All scripts |
| `VENV_PATH` | `/home/metis_env` | All scripts |
| `DATA_BASE` | `/mnt/d/simData_202602/output` | 02, 04 (use `--data-dir`) |

The `--data-dir` argument overrides `DATA_BASE` in scripts 02 and 04.

## Expected Test Results

```
1060 passed, 2 skipped, 389 deselected, 36 errors
```

The 36 errors are expected: EDPS cannot process the CHOPHOME and PUPIL calibration data.

## Known Limitations

- The `metis_cal_chophome` and `metis_pupil_imaging` recipes have no matching EDPS jobs
  with the current simulated data (EDPS classifies CHOPHOME and PUPIL raw files as "NONE").
  This results in ~36 test errors from missing SOF files.
- The `metis_lm_ravc_post` job consistently fails during the hciRavcLM workflow.
  This does not affect test results.
