#!/bin/bash
# =============================================================================
# METIS Pipeline - WSL Setup Script
# =============================================================================
# Installs system dependencies, creates a Python venv, installs Python packages,
# and configures EDPS on a fresh Ubuntu 24.04 WSL instance.
#
# Usage (from inside WSL, as root):
#   sudo bash /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/01_setup_wsl.sh
#
# Or from PowerShell:
#   wsl -d Ubuntu-24.04 -- sudo bash /mnt/d/Repos/METIS_Pipeline/toolbox/windows-wsl/01_setup_wsl.sh
# =============================================================================
set -euo pipefail

# ---------------------------------------------------------------------------
# Configuration - EDIT THESE PATHS for your environment
# ---------------------------------------------------------------------------
REPO_PATH="/mnt/d/Repos/METIS_Pipeline"    # Path to METIS_Pipeline repo in WSL
VENV_PATH="/home/metis_env"                  # Where to create the Python venv

# ---------------------------------------------------------------------------
# Step 1: Install system dependencies
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== Step 1: Installing system packages ==="
echo "=========================================="

apt-get update
apt-get install -y \
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

echo "System packages installed."

# ---------------------------------------------------------------------------
# Step 2: Create Python virtual environment
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== Step 2: Creating Python venv ==="
echo "=========================================="

python3 -m venv "${VENV_PATH}"
source "${VENV_PATH}/bin/activate"
pip install --upgrade pip

echo "Python venv created at ${VENV_PATH}"
echo "Python version: $(python --version)"

# ---------------------------------------------------------------------------
# Step 3: Install Python packages
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== Step 3: Installing Python packages ==="
echo "=========================================="

# IMPORTANT: Install pycpl from ivh repo (not ESO FTP)
# The ivh version (1.0.3.post4) includes fixes required by the pipeline
pip install --extra-index-url https://ivh.github.io/pycpl/simple/ pycpl==1.0.3.post4

# Install remaining ESO packages
pip install --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries \
    pyesorex edps adari_core

# Install pytest for running tests
pip install pytest

echo ""
echo "Installed packages:"
pip list | grep -iE 'pycpl|pyesorex|edps|adari|pytest'

# ---------------------------------------------------------------------------
# Step 4: Configure EDPS
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== Step 4: Configuring EDPS ==="
echo "=========================================="

# Create required directories for root (scripts run via sudo)
mkdir -p /root/.edps /root/.esorex
mkdir -p /tmp/EDPS_data && chmod 1777 /tmp/EDPS_data

# Also create for the regular user (if running interactively later)
if [ -n "${SUDO_USER:-}" ] && [ "${SUDO_USER}" != "root" ]; then
    USER_HOME=$(eval echo "~${SUDO_USER}")
    mkdir -p "${USER_HOME}/.edps" "${USER_HOME}/.esorex"
fi

# Copy config templates from repo
cp "${REPO_PATH}/toolbox/config/DOTedps/logging.yaml" /root/.edps/
cp "${REPO_PATH}/toolbox/config/DOTesorex/esorex.rc" /root/.esorex/

# Create application.properties with correct paths
cat > /root/.edps/application.properties << EDPSEOF
[server]
host=localhost
port=5000

[application]
workflow_dir=${REPO_PATH}/metisp/workflows
esorex_path=pyesorex
base_dir=/tmp/EDPS_data
dummy=False
continue_on_error=False
processes=1
cores=1
default_omp_threads=1
ordering=dfs
output_prefix=
resume_on_startup=False

[executor]
esorex_path=pyesorex
pipeline_path=
genreport_path=genreport

[generator]
calibrations_config_file=
parameters_config_file=
association_preference=raw_per_quality_level
breakpoints_url=
meta_workflow=

[repository]
truncate=False
local=True
path=/tmp/EDPS_data/db.json
type=caching
flush_size=10
flush_timeout=60
min_disk_space_mb=100

[cleanup]
enabled=False

[packager]
package_base_dir=
mode=copy
pattern=\$DATASET/\$TIMESTAMP/\$object\$_\$pro.catg\$.\$EXT
categories=
EDPSEOF

# Copy config to regular user's home too (if running via sudo)
if [ -n "${SUDO_USER:-}" ] && [ "${SUDO_USER}" != "root" ]; then
    USER_HOME=$(eval echo "~${SUDO_USER}")
    cp -r /root/.edps/* "${USER_HOME}/.edps/"
    cp -r /root/.esorex/* "${USER_HOME}/.esorex/"
    chown -R "${SUDO_USER}:${SUDO_USER}" "${USER_HOME}/.edps" "${USER_HOME}/.esorex"
    echo "EDPS configured for both root and ${SUDO_USER}."
else
    echo "EDPS configured."
fi
echo "  Config: /root/.edps/application.properties"
echo "  Workflow dir: ${REPO_PATH}/metisp/workflows"

# ---------------------------------------------------------------------------
# Step 5: Verify installation
# ---------------------------------------------------------------------------
echo ""
echo "=========================================="
echo "=== Step 5: Verifying installation ==="
echo "=========================================="

export PYESOREX_PLUGIN_DIR="${REPO_PATH}/metisp/pymetis/src/pymetis/recipes"
export PYCPL_RECIPE_DIR="${REPO_PATH}/metisp/pyrecipes/"
export PYTHONPATH="${REPO_PATH}/metisp/pymetis/src/"

echo "Testing pycpl import..."
python -c "import cpl; print(f'  pycpl OK: {cpl.__version__}')"

echo "Testing pyesorex..."
pyesorex --recipes 2>&1 | head -5

echo ""
echo "=========================================="
echo "=== Setup complete ==="
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Run EDPS workflows: bash ${REPO_PATH}/toolbox/windows-wsl/02_run_edps_workflows.sh"
echo "  2. Create SOF files:   bash ${REPO_PATH}/toolbox/windows-wsl/03_create_sof_files.sh"
echo "  3. Run tests:          bash ${REPO_PATH}/toolbox/windows-wsl/04_run_tests.sh"
