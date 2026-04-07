#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Run as normal user 'metis'

# Temporary fix! Pin pybind version so that PyCPL 1.0.3 compiles successfully until PyCPL is updated
pip install "pybind11<3.0"

pip install --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries pycpl pyesorex edps adari_core
