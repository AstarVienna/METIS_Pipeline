#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Run as normal user 'metis'

# Temporary fix! Pin pybind version so that PyCPL 1.0.3 compiles successfully until PyCPL is updated
# Pre-install pybind11 < 3.0 to avoid incompatibility with pycpl 1.0.3,
# then build pycpl with --no-build-isolation so it uses this version
# instead of pip's isolated build environment picking up pybind11 3.x.
pip install "pybind11>=2.8,<3.0"
pip install --no-build-isolation --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries pycpl
pip install --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries pyesorex edps adari_core
