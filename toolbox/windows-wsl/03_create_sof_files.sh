#!/bin/bash
# =============================================================================
# METIS Pipeline - Create SOF Files for Tests
# =============================================================================
# Maps EDPS task directories to test-expected SOF naming conventions and fixes
# internal FITS paths to point to the accumulated output directory.
#
# The test suite expects SOF files in $SOF_DIR with names like:
#   metis_det_dark.lm.sof      (not metis_lm_img_dark/input.sof)
#   metis_lm_img_flat.lamp.sof (not metis_lm_img_flat/input.sof)
#
# This script also rewrites paths inside the SOF files:
#   /tmp/EDPS_data/METIS/  ->  /tmp/sof_accumulated/
# so that references to intermediate FITS products resolve correctly.
#
# Usage (from inside WSL):
#   bash /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/03_create_sof_files.sh
#
# Prerequisites:
#   - 02_run_edps_workflows.sh has been run successfully
#   - /tmp/sof_accumulated/ contains the accumulated EDPS output
# =============================================================================
set -euo pipefail

REPO_PATH="/mnt/d/Repos/METIS_Pipeline"

# Where test SOF files will be created
SOF_DIR=/tmp/sof_for_tests
rm -rf "${SOF_DIR}"
mkdir -p "${SOF_DIR}"

# Where EDPS output was accumulated
ACCUM=/tmp/sof_accumulated

# ---------------------------------------------------------------------------
# Helper: copy and fix paths in SOF file
# ---------------------------------------------------------------------------
copy_sof() {
    local edps_task="$1"   # EDPS task directory name
    local test_name="$2"   # Test-expected SOF name (without .sof extension)
    local sof_file
    sof_file=$(find "${ACCUM}/${edps_task}" -name "input.sof" 2>/dev/null | head -1)
    if [ -n "${sof_file}" ]; then
        # Copy and rewrite EDPS paths to accumulator paths
        sed 's|/tmp/EDPS_data/METIS/|/tmp/sof_accumulated/|g' "${sof_file}" \
            > "${SOF_DIR}/${test_name}.sof"
        echo "  ${test_name}.sof <- ${edps_task}"
    else
        echo "  MISSING: ${test_name}.sof (no ${edps_task} task found)"
    fi
}

echo "=== Creating SOF files ==="
echo "  Source: ${ACCUM}"
echo "  Target: ${SOF_DIR}"
echo ""

# --- Dark recipes ---
# EDPS splits by detector mode, tests expect metis_det_dark.<band>
echo "--- Dark ---"
copy_sof "metis_lm_img_dark" "metis_det_dark.lm"
copy_sof "metis_n_img_dark" "metis_det_dark.n"
copy_sof "metis_ifu_dark" "metis_det_dark.ifu"

# --- Lingain recipes ---
echo "--- Lingain ---"
copy_sof "metis_lm_img_lingain" "metis_det_lingain.lm"
copy_sof "metis_n_img_lingain" "metis_det_lingain.n"
copy_sof "metis_ifu_lingain" "metis_det_lingain.ifu"

# --- LM Image flat ---
# Tests expect both lamp and twilight variants (same SOF, different names)
echo "--- LM Img Flat ---"
copy_sof "metis_lm_img_flat" "metis_lm_img_flat.lamp"
copy_sof "metis_lm_img_flat" "metis_lm_img_flat.twilight"

# --- N Image flat ---
echo "--- N Img Flat ---"
copy_sof "metis_n_img_flat" "metis_n_img_flat.lamp"
copy_sof "metis_n_img_flat" "metis_n_img_flat.twilight"

# --- LM Image background ---
echo "--- LM Img Background ---"
copy_sof "metis_lm_img_background_sci" "metis_lm_img_background.sci"
copy_sof "metis_lm_img_background_std" "metis_lm_img_background.std"

# --- LM Image basic reduce ---
echo "--- LM Img Basic Reduce ---"
copy_sof "metis_lm_img_basic_reduce_sci" "metis_lm_img_basic_reduce.sci"
copy_sof "metis_lm_img_basic_reduce_sky" "metis_lm_img_basic_reduce.sky"
copy_sof "metis_lm_img_basic_reduce_std" "metis_lm_img_basic_reduce.std"

# --- LM Image calibrate and postprocess ---
echo "--- LM Img Calibrate/Postprocess ---"
copy_sof "metis_lm_img_calib" "metis_lm_img_calibrate"
copy_sof "metis_lm_img_coadd" "metis_lm_img_sci_postprocess"
copy_sof "metis_lm_img_standard_flux" "metis_lm_img_std_process"
copy_sof "metis_lm_img_distortion" "metis_lm_img_distortion"

# --- N Image chopnod ---
echo "--- N Img Chopnod ---"
copy_sof "metis_n_img_chopnod_sci" "metis_n_img_chopnod.sci"
copy_sof "metis_n_img_chopnod_std" "metis_n_img_chopnod.std"

# --- N Image calibrate, restore, std ---
echo "--- N Img Calibrate/Restore ---"
copy_sof "metis_n_img_calib" "metis_n_img_calibrate"
copy_sof "metis_n_img_restore" "metis_n_img_restore"
copy_sof "metis_n_img_standard_flux" "metis_n_img_std_process"
copy_sof "metis_n_img_distortion" "metis_n_img_distortion"

# --- IFU recipes ---
echo "--- IFU ---"
copy_sof "metis_ifu_distortion" "metis_ifu_distortion"
copy_sof "metis_ifu_rsrf" "metis_ifu_rsrf"
copy_sof "metis_ifu_wavecal" "metis_ifu_wavecal"
copy_sof "metis_ifu_sci_reduce" "metis_ifu_reduce.sci"
copy_sof "metis_ifu_std_reduce" "metis_ifu_reduce.std"
copy_sof "metis_ifu_sci_telluric" "metis_ifu_telluric.sci"
copy_sof "metis_ifu_std_telluric" "metis_ifu_telluric.std"
copy_sof "metis_ifu_calibrate" "metis_ifu_calibrate"
copy_sof "metis_ifu_postprocess" "metis_ifu_postprocess"

# --- LM LSS recipes ---
echo "--- LM LSS ---"
copy_sof "metis_lm_lss_adc_slitloss" "metis_lm_adc_slitloss"
copy_sof "metis_lm_lss_rsrf" "metis_lm_lss_rsrf"
copy_sof "metis_lm_lss_trace" "metis_lm_lss_trace"
copy_sof "metis_lm_lss_wave" "metis_lm_lss_wave"
copy_sof "metis_lm_lss_sci" "metis_lm_lss_sci"
copy_sof "metis_lm_lss_std" "metis_lm_lss_std"
copy_sof "metis_lm_lss_mf_model" "metis_lm_lss_mf_model"
copy_sof "metis_lm_lss_mf_calctrans" "metis_lm_lss_mf_calctrans"
copy_sof "metis_lm_lss_mf_correct" "metis_lm_lss_mf_correct"

# --- N LSS recipes ---
echo "--- N LSS ---"
copy_sof "metis_n_adc_slitloss" "metis_n_adc_slitloss"
copy_sof "metis_n_lss_rsrf" "metis_n_lss_rsrf"
copy_sof "metis_n_lss_trace" "metis_n_lss_trace"
copy_sof "metis_n_lss_sci" "metis_n_lss_sci"
copy_sof "metis_n_lss_std" "metis_n_lss_std"
copy_sof "metis_n_lss_mf_model" "metis_n_lss_mf_model"
copy_sof "metis_n_lss_mf_calctrans" "metis_n_lss_mf_calctrans"
copy_sof "metis_n_lss_mf_correct" "metis_n_lss_mf_correct"

# --- Calibration recipes (may not have data) ---
echo "--- Calibration ---"
copy_sof "metis_cal_chophome" "metis_cal_chophome"
copy_sof "metis_pupil_imaging" "metis_pupil_imaging.lm"
copy_sof "metis_pupil_imaging" "metis_pupil_imaging.n"

# ---------------------------------------------------------------------------
# Summary and verification
# ---------------------------------------------------------------------------
echo ""
echo "=== SOF files created ==="
echo "Total: $(ls ${SOF_DIR}/*.sof 2>/dev/null | wc -l) files"
echo ""
ls -la "${SOF_DIR}/"

# Verify that all FITS files referenced in SOFs actually exist
echo ""
echo "=== Verifying FITS file references ==="
missing=0
total=0
for sof in ${SOF_DIR}/*.sof; do
    while read -r line; do
        path=$(echo "$line" | awk '{print $1}')
        if [ -n "$path" ] && [ ! -f "$path" ]; then
            echo "  MISSING: $path (in $(basename $sof))"
            missing=$((missing + 1))
        fi
        total=$((total + 1))
    done < "$sof"
done
echo "Total references: ${total}, Missing: ${missing}"

if [ "${missing}" -eq 0 ]; then
    echo ""
    echo "All FITS references verified OK."
else
    echo ""
    echo "WARNING: ${missing} FITS files are missing. Tests will fail for affected recipes."
fi

echo ""
echo "Next step: bash ${REPO_PATH}/toolbox/windows-wsl/04_run_tests.sh"
