#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Run as normal user 'metis'
export PYCPL_RECIPE_DIR="{script_dir}/../metisp/pyrecipes/"
pip install --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries pycpl pyesorex edps adari_core
