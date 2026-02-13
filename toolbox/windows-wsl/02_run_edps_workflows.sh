#!/bin/bash
# =============================================================================
# METIS Pipeline - Run All EDPS Workflows
# =============================================================================
# Runs all EDPS workflows on simulated test data and accumulates the complete
# output (FITS products + SOF files) for use by the test suite.
#
# IMPORTANT: This script accumulates ALL EDPS output files, not just SOFs.
# The SOF files reference intermediate FITS products (MASTER_DARK, GAIN_MAP, etc.)
# from earlier pipeline stages. These must be preserved or tests will fail with
# FileIOError when trying to open the referenced FITS files.
#
# Usage (from inside WSL):
#   bash 02_run_edps_workflows.sh [--data-dir /path/to/data/output]
#
# Arguments:
#   --data-dir  Path to the extracted simulated data (containing imgLM/, imgN/, etc.)
#               Default: /mnt/d/simData_202602/output
#
# Takes approximately 15 minutes to complete all workflows.
# =============================================================================
set -euo pipefail

# ---------------------------------------------------------------------------
# Configuration - EDIT THESE PATHS for your environment
# ---------------------------------------------------------------------------
REPO_PATH="/mnt/d/Repos/METIS_Pipeline"
VENV_PATH="/home/metis_env"
DATA_BASE="/mnt/d/simData_202602/output"     # Root of extracted simulated FITS data

# ---------------------------------------------------------------------------
# Parse command-line arguments
# ---------------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        --data-dir)
            DATA_BASE="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1"
            echo "Usage: $0 [--data-dir /path/to/data/output]"
            exit 1
            ;;
    esac
done

# Accumulator: stores ALL EDPS output (FITS + SOFs) across workflow runs
# MUST NOT start with /tmp/EDPS to avoid being wiped by rm -rf /tmp/EDPS*
ACCUM="/tmp/sof_accumulated"

# ---------------------------------------------------------------------------
# Setup
# ---------------------------------------------------------------------------
source "${VENV_PATH}/bin/activate"
export PYESOREX_PLUGIN_DIR="${REPO_PATH}/metisp/pymetis/src/pymetis/recipes"
export PYCPL_RECIPE_DIR="${REPO_PATH}/metisp/pyrecipes/"
export PYTHONPATH="${REPO_PATH}/metisp/pymetis/src/"

# Clean up from any previous run (may need root ownership cleanup)
rm -rf "${ACCUM}" /tmp/EDPS* /tmp/METIS*
mkdir -p "${ACCUM}"
mkdir -p /tmp/EDPS_data && chmod 1777 /tmp/EDPS_data

# ---------------------------------------------------------------------------
# Workflow runner function
# ---------------------------------------------------------------------------
run_workflow() {
    local mode="$1"       # Data subdirectory name (e.g. "imgLM")
    local workflow="$2"   # EDPS workflow name (e.g. "metis.metis_lm_img_wkf")
    local data="${DATA_BASE}/${mode}"

    echo ""
    echo "=========================================="
    echo "=== Running ${workflow} with data from ${mode} ==="
    echo "=========================================="

    # Clean shutdown and remove previous EDPS state
    edps --shutdown 2>/dev/null || true
    rm -rf /tmp/EDPS* /tmp/METIS*
    sleep 2

    # Classify data files
    edps -w metis.metis_wkf -i "${data}" -c
    sleep 1

    # Run the workflow (|| true to continue on failures)
    edps -w "${workflow}" -i "${data}" -o /tmp -m science || true
    echo "=== ${mode} Done ==="

    # Accumulate ENTIRE EDPS output tree (FITS products + SOFs)
    if [ -d /tmp/EDPS_data/METIS ]; then
        echo "  Accumulating EDPS output..."
        cp -r /tmp/EDPS_data/METIS/* "${ACCUM}/" 2>/dev/null || true
        echo "  Saved $(find /tmp/EDPS_data/METIS -name '*.fits' | wc -l) FITS files"
        echo "  Saved $(find /tmp/EDPS_data/METIS -name 'input.sof' | wc -l) SOF files"
    fi
}

# ---------------------------------------------------------------------------
# Run all workflows
# ---------------------------------------------------------------------------
echo "Starting EDPS workflow runs..."
echo "Data location: ${DATA_BASE}"
echo "Accumulator: ${ACCUM}"

# LM Image (12 jobs)
run_workflow "imgLM" "metis.metis_lm_img_wkf"

# N Image (11 jobs)
run_workflow "imgN" "metis.metis_n_img_wkf"

# IFU (11 jobs)
run_workflow "ifu" "metis.metis_ifu_wkf"

# LM Long-Slit Spectroscopy (11 jobs)
run_workflow "lssLM" "metis.metis_lm_lss_wkf"

# N Long-Slit Spectroscopy (11 jobs)
run_workflow "lssN" "metis.metis_n_lss_wkf"

# Calibration - chophome (0 jobs expected - EDPS classifies CHOPHOME as NONE)
run_workflow "Calib" "metis.metis_chophome_wkf"

# Calibration - pupil imaging (0 jobs expected - EDPS classifies PUPIL as NONE)
echo ""
echo "=== Trying pupil imaging workflow with Calib data ==="
edps --shutdown 2>/dev/null || true
rm -rf /tmp/EDPS* /tmp/METIS*
sleep 2
edps -w metis.metis_wkf -i "${DATA_BASE}/Calib" -c
sleep 1
edps -w metis.metis_pupil_imaging_wkf -i "${DATA_BASE}/Calib" -o /tmp -m science \
    || echo "=== Pupil imaging: no matching jobs ==="
if [ -d /tmp/EDPS_data/METIS ]; then
    cp -r /tmp/EDPS_data/METIS/* "${ACCUM}/" 2>/dev/null || true
fi

# HCI APP LM (7 jobs)
run_workflow "hciAppLM" "metis.metis_lm_app_wkf"

# HCI RAVC LM (12 completed + 1 failed metis_lm_ravc_post)
run_workflow "hciRavcLM" "metis.metis_lm_ravc_wkf"

# HCI RAVC IFU (7 jobs)
run_workflow "hciRavcIfu" "metis.metis_ifu_wkf"

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== All workflows complete ==="
echo "=========================================="
echo ""
echo "Accumulated contents in ${ACCUM}:"
echo "  Task directories: $(ls -d ${ACCUM}/*/ 2>/dev/null | wc -l)"
echo "  FITS files: $(find ${ACCUM} -name '*.fits' | wc -l)"
echo "  SOF files: $(find ${ACCUM} -name 'input.sof' | wc -l)"
echo ""
echo "Task directories:"
ls "${ACCUM}/"
echo ""
echo "Next step: bash ${REPO_PATH}/toolbox/windows-wsl/03_create_sof_files.sh"
