#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Set environment variables.
source "${script_dir}/set_environment.sh"

# Enter root of this checkout.
pushd "${script_dir}/.."

mkdir -p "${HOME}/EDPS_data"
mkdir -p "/tmp/EDPS_data"
mkdir -p "${HOME}/.edps"
mkdir -p "${HOME}/.esorex"

cp -av ./toolbox/config/DOTedps/* "${HOME}/.edps"
cp -av ./toolbox/config/DOTesorex/* "${HOME}/.esorex"

