# METIS Pipeline - Windows WSL Installation Guide

Complete walkthrough for installing and testing the METIS Pipeline on Windows via WSL.

Tested on: Windows 11, WSL2, Ubuntu 24.04.4 LTS, Python 3.12.3.

> **Note**: The bash scripts in this directory have Windows `\r\n` line endings because they
> live on a Windows filesystem. When running them from WSL, pipe through `sed 's/\r$//'` or
> use `dos2unix` first. The [Quick Start](README.md#quick-start) commands already handle this.

## Table of Contents

1. [Create a Fresh WSL Instance](#1-create-a-fresh-wsl-instance)
2. [Install System Dependencies](#2-install-system-dependencies)
3. [Create Python Virtual Environment](#3-create-python-virtual-environment)
4. [Install Python Packages](#4-install-python-packages)
5. [Configure EDPS](#5-configure-edps)
6. [Prepare Test Data](#6-prepare-test-data)
7. [Run EDPS Workflows](#7-run-edps-workflows)
8. [Create SOF Files for Tests](#8-create-sof-files-for-tests)
9. [Run the Test Suite](#9-run-the-test-suite)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. Create a Fresh WSL Instance

From PowerShell (run as Administrator if needed):

```powershell
# Optional: remove existing Ubuntu instance (WARNING: destroys all data in it)
wsl --unregister Ubuntu-24.04

# Install a fresh Ubuntu 24.04
wsl --install Ubuntu-24.04
```

After installation, Ubuntu will prompt you to create a user account. Complete the setup,
then all subsequent commands should be run **inside the WSL instance**.

## 2. Install System Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    wget gcc automake autogen libtool gsl-bin libgsl-dev \
    libfftw3-bin libfftw3-dev fftw-dev \
    curl bzip2 less subversion git cppcheck lcov valgrind \
    zlib1g zlib1g-dev \
    liberfa1 liberfa-dev \
    libcurl4-openssl-dev libcurl4 \
    tmux ripgrep file \
    libcfitsio-bin libcfitsio-dev \
    wcslib-dev wcslib-tools \
    libcpl-dev \
    python3-astropy python3-matplotlib python3-numpy \
    perl cmake \
    graphviz \
    python3-pip python3-full python3-venv
```

These are the same packages listed in `toolbox/install_dependencies_ubuntu.sh`, minus
optional GUI tools (meld, jupyter, editors) that aren't needed for headless testing.

## 3. Create Python Virtual Environment

```bash
python3 -m venv /home/metis_env
source /home/metis_env/bin/activate
pip install --upgrade pip
```

The venv is placed at `/home/metis_env` to keep it on the native Linux filesystem
(not on `/mnt/...`) for better I/O performance.

## 4. Install Python Packages

**Important**: `pycpl` must be installed from the [ivh repository](https://ivh.github.io/pycpl/simple/),
not from the ESO FTP. The ivh version (1.0.3.post4) includes fixes needed for the pipeline.

```bash
# Install pycpl from ivh repo FIRST
pip install --extra-index-url https://ivh.github.io/pycpl/simple/ pycpl==1.0.3.post4

# Install remaining ESO packages from ESO FTP
pip install --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries \
    pyesorex edps adari_core

# Install pytest for running tests
pip install pytest
```

### Verify installation

```bash
python -c "import cpl; print(f'pycpl version: {cpl.__version__}')"
pyesorex --recipes    # Should list METIS recipes
edps --version        # Should show EDPS version
```

### Expected package versions (as tested)

| Package | Version |
|---------|---------|
| pycpl | 1.0.3.post4 |
| pyesorex | 1.0.3 |
| edps | 1.7.0 |
| adari_core | 4.2.0 |
| pytest | 9.0.2+ |

## 5. Configure EDPS

EDPS needs configuration files in `~/.edps/` and `~/.esorex/`. The repo provides
templates, but they need to be modified for your environment.

### Option A: Use the provided script (recommended)

```bash
# Set the path to your repo checkout (adjust for your Windows drive letter)
REPO=/mnt/d/Repos/METIS_Pipeline

# Copy config templates
mkdir -p ~/.edps ~/.esorex /tmp/EDPS_data
cp ${REPO}/toolbox/config/DOTedps/* ~/.edps/
cp ${REPO}/toolbox/config/DOTesorex/* ~/.esorex/
```

Then edit `~/.edps/application.properties` to set the correct paths:

```ini
# Set the workflow directory to the absolute path of the workflows folder
workflow_dir=/mnt/d/Repos/METIS_Pipeline/metisp/workflows

# Use pyesorex (not esorex) as the recipe executor
esorex_path=pyesorex

# Leave breakpoints_url empty (required for offline use)
breakpoints_url=
```

### Option B: Manual configuration

Create `~/.edps/application.properties` with at minimum:

```ini
[server]
host=localhost
port=5000

[application]
workflow_dir=/mnt/d/Repos/METIS_Pipeline/metisp/workflows

[executor]
esorex_path=pyesorex
base_dir=/tmp/EDPS_data

[repository]
path=/tmp/EDPS_data/db.json
type=caching
```

## 6. Prepare Test Data

The simulated FITS data should be extracted from the tar file into a directory accessible
from WSL. The expected directory structure is:

```
/mnt/d/simData_202602/output/
    imgLM/       # LM-band imaging data (~46 FITS files)
    imgN/        # N-band imaging data (~46 FITS files)
    ifu/         # IFU spectroscopy data (~45 FITS files)
    lssLM/       # LM long-slit spectroscopy (~62 FITS files)
    lssN/        # N long-slit spectroscopy (~58 FITS files)
    Calib/       # Calibration data (~44 FITS files)
    hciAppLM/    # HCI APP LM data (~46 FITS files)
    hciRavcLM/   # HCI RAVC LM data (~56 FITS files)
    hciRavcIfu/  # HCI RAVC IFU data (~50 FITS files)
```

You can override the data path at runtime using the `--data-dir` argument (see below),
or edit the `DATA_BASE` variable at the top of the scripts.

## 7. Run EDPS Workflows

EDPS workflows process the raw FITS data and produce:
- Intermediate FITS products (MASTER_DARK, GAIN_MAP, LINEARITY, etc.)
- SOF (Set-Of-Frames) files that describe the input/output relationships

### Environment variables

Before running EDPS, set these environment variables:

```bash
source /home/metis_env/bin/activate
export PYESOREX_PLUGIN_DIR=/mnt/d/Repos/METIS_Pipeline/metisp/pymetis/src/pymetis/recipes
export PYCPL_RECIPE_DIR=/mnt/d/Repos/METIS_Pipeline/metisp/pyrecipes/
export PYTHONPATH=/mnt/d/Repos/METIS_Pipeline/metisp/pymetis/src/
```

### Running all workflows

Use the provided script `02_run_edps_workflows.sh` with `--data-dir` to point to your data:

```bash
# Strip CRLF and pass --data-dir
sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/02_run_edps_workflows.sh \
    | bash -s -- --data-dir /mnt/d/simData_202602/output
```

Or run manually:

```bash
# For each data mode and its corresponding workflow:
DATA_BASE=/mnt/d/simData_202602/output

# Pattern for each workflow:
edps --shutdown 2>/dev/null || true
rm -rf /tmp/EDPS* /tmp/METIS*
sleep 2
edps -w metis.metis_wkf -i ${DATA_BASE}/imgLM -c    # Classify data
sleep 1
edps -w metis.metis_lm_img_wkf -i ${DATA_BASE}/imgLM -o /tmp -m science  # Run workflow
```

### Critical: Accumulate ALL output between workflows

Each workflow run writes to `/tmp/EDPS_data/METIS/`. Before starting the next workflow
(which cleans `/tmp/EDPS*`), you must copy the **entire output tree** to an accumulator:

```bash
ACCUM=/tmp/sof_accumulated
cp -r /tmp/EDPS_data/METIS/* ${ACCUM}/
```

**Why?** Later workflows' SOF files reference intermediate FITS products from earlier
workflows (e.g., `metis_lm_lss_rsrf` needs `MASTER_DARK` from `metis_lm_lss_dark`).
If you only save the SOF files, the FITS paths they contain will be broken.

### Workflow → Data directory mapping

| Data directory | EDPS workflow | Expected jobs |
|----------------|---------------|---------------|
| imgLM | metis.metis_lm_img_wkf | 12 |
| imgN | metis.metis_n_img_wkf | 11 |
| ifu | metis.metis_ifu_wkf | 11 |
| lssLM | metis.metis_lm_lss_wkf | 11 |
| lssN | metis.metis_n_lss_wkf | 11 |
| Calib | metis.metis_chophome_wkf | 0 (*) |
| Calib | metis.metis_pupil_imaging_wkf | 0 (*) |
| hciAppLM | metis.metis_lm_app_wkf | 7 |
| hciRavcLM | metis.metis_lm_ravc_wkf | 12 (+1 failed) |
| hciRavcIfu | metis.metis_ifu_wkf | 7 |

(*) Calib workflows produce 0 jobs because EDPS classifies CHOPHOME and PUPIL raw files as "NONE".

## 8. Create SOF Files for Tests

The test suite expects SOF files in a flat directory (`$SOF_DIR`) with specific naming
conventions that differ from EDPS's task directory names.

Use `03_create_sof_files.sh` to:
1. Map EDPS task names to test-expected SOF names
2. Fix internal FITS paths from `/tmp/EDPS_data/METIS/` to `/tmp/sof_accumulated/`
3. Verify all referenced FITS files exist

### EDPS task → Test SOF name mapping

| EDPS Task | Test SOF Name |
|-----------|---------------|
| metis_lm_img_dark | metis_det_dark.lm.sof |
| metis_n_img_dark | metis_det_dark.n.sof |
| metis_ifu_dark | metis_det_dark.ifu.sof |
| metis_lm_img_lingain | metis_det_lingain.lm.sof |
| metis_n_img_lingain | metis_det_lingain.n.sof |
| metis_ifu_lingain | metis_det_lingain.ifu.sof |
| metis_lm_img_flat | metis_lm_img_flat.lamp.sof, metis_lm_img_flat.twilight.sof |
| metis_n_img_flat | metis_n_img_flat.lamp.sof, metis_n_img_flat.twilight.sof |
| metis_lm_img_background_sci | metis_lm_img_background.sci.sof |
| metis_lm_img_background_std | metis_lm_img_background.std.sof |
| metis_lm_img_basic_reduce_sci | metis_lm_img_basic_reduce.sci.sof |
| metis_lm_img_basic_reduce_sky | metis_lm_img_basic_reduce.sky.sof |
| metis_lm_img_basic_reduce_std | metis_lm_img_basic_reduce.std.sof |
| metis_lm_img_calib | metis_lm_img_calibrate.sof |
| metis_lm_img_coadd | metis_lm_img_sci_postprocess.sof |
| metis_lm_img_standard_flux | metis_lm_img_std_process.sof |
| metis_lm_img_distortion | metis_lm_img_distortion.sof |
| metis_n_img_chopnod_sci | metis_n_img_chopnod.sci.sof |
| metis_n_img_chopnod_std | metis_n_img_chopnod.std.sof |
| metis_n_img_calib | metis_n_img_calibrate.sof |
| metis_n_img_restore | metis_n_img_restore.sof |
| metis_n_img_standard_flux | metis_n_img_std_process.sof |
| metis_n_img_distortion | metis_n_img_distortion.sof |
| metis_ifu_distortion | metis_ifu_distortion.sof |
| metis_ifu_rsrf | metis_ifu_rsrf.sof |
| metis_ifu_wavecal | metis_ifu_wavecal.sof |
| metis_ifu_sci_reduce | metis_ifu_reduce.sci.sof |
| metis_ifu_std_reduce | metis_ifu_reduce.std.sof |
| metis_ifu_sci_telluric | metis_ifu_telluric.sci.sof |
| metis_ifu_std_telluric | metis_ifu_telluric.std.sof |
| metis_ifu_calibrate | metis_ifu_calibrate.sof |
| metis_ifu_postprocess | metis_ifu_postprocess.sof |
| metis_lm_lss_adc_slitloss | metis_lm_adc_slitloss.sof |
| metis_lm_lss_rsrf | metis_lm_lss_rsrf.sof |
| metis_lm_lss_trace | metis_lm_lss_trace.sof |
| metis_lm_lss_wave | metis_lm_lss_wave.sof |
| metis_lm_lss_sci | metis_lm_lss_sci.sof |
| metis_lm_lss_std | metis_lm_lss_std.sof |
| metis_lm_lss_mf_model | metis_lm_lss_mf_model.sof |
| metis_lm_lss_mf_calctrans | metis_lm_lss_mf_calctrans.sof |
| metis_lm_lss_mf_correct | metis_lm_lss_mf_correct.sof |
| metis_n_adc_slitloss | metis_n_adc_slitloss.sof |
| metis_n_lss_rsrf | metis_n_lss_rsrf.sof |
| metis_n_lss_trace | metis_n_lss_trace.sof |
| metis_n_lss_sci | metis_n_lss_sci.sof |
| metis_n_lss_std | metis_n_lss_std.sof |
| metis_n_lss_mf_model | metis_n_lss_mf_model.sof |
| metis_n_lss_mf_calctrans | metis_n_lss_mf_calctrans.sof |
| metis_n_lss_mf_correct | metis_n_lss_mf_correct.sof |

## 9. Run the Test Suite

```bash
# Using the provided script with --data-dir:
sed 's/\r$//' /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/04_run_tests.sh \
    | bash -s -- --data-dir /mnt/d/simData_202602/output
```

Or manually:

```bash
source /home/metis_env/bin/activate
export PYESOREX_PLUGIN_DIR=/mnt/d/Repos/METIS_Pipeline/metisp/pymetis/src/pymetis/recipes
export PYCPL_RECIPE_DIR=/mnt/d/Repos/METIS_Pipeline/metisp/pyrecipes/
export PYTHONPATH=/mnt/d/Repos/METIS_Pipeline/metisp/pymetis/src/
export SOF_DIR=/tmp/sof_for_tests
export SOF_DATA=/mnt/d/simData_202602/output/imgLM

cd /mnt/d/Repos/METIS_Pipeline/metisp/pymetis
python -m pytest src/pymetis/tests/ -v --tb=short \
    -m "not external and not edps and not pyesorex"
```

### Expected results

```
1060 passed, 2 skipped, 389 deselected, 36 errors
```

- **1060 passed**: All recipe, inputset, product, and configuration tests
- **2 skipped**: Tests marked as skip in the code
- **389 deselected**: Tests marked `external`, `edps`, or `pyesorex` (require full pipeline execution)
- **36 errors**: Missing SOF files for `metis_cal_chophome` (15) and `metis_pupil_imaging` (21)

The 36 errors are expected because EDPS does not produce jobs for the Calib data
(CHOPHOME and PUPIL raw files are classified as "NONE").

## 10. Troubleshooting

### CRLF line endings

Scripts written on Windows may have `\r\n` line endings that cause `bash` errors like
`/bin/bash^M: bad interpreter`. Fix with:

```bash
tr -d '\r' < script.sh > /tmp/script.sh && bash /tmp/script.sh
```

Or use `dos2unix` if installed.

### Git Bash path conversion

If running WSL commands from Git Bash (not PowerShell), paths starting with `/` get
converted to Windows paths. Disable this with:

```bash
MSYS_NO_PATHCONV=1 wsl -d Ubuntu-24.04 -- bash -c "command"
```

### EDPS won't start

If EDPS fails to start or connect:

```bash
edps --shutdown 2>/dev/null || true
rm -rf /tmp/EDPS* /tmp/METIS*
sleep 2
# Then retry
```

### pycpl import errors

If `import cpl` fails, ensure you installed pycpl from the ivh repo, not the ESO FTP:

```bash
pip install --extra-index-url https://ivh.github.io/pycpl/simple/ pycpl==1.0.3.post4
```

The ESO FTP version may not include required fixes.

### Slow file I/O on /mnt drives

WSL file access to Windows drives (`/mnt/c/`, `/mnt/d/`) is significantly slower than
native Linux paths. The venv and EDPS output are placed on the Linux filesystem (`/home/`,
`/tmp/`) for performance. Only the repo source and FITS data are read from `/mnt/`.
