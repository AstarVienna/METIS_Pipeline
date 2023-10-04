#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Set environment variables.
source "${script_dir}/set_environment.sh"

# Enter root of this checkout.
pushd "${script_dir}/.."

# Configure, make and install metisp.
pushd metisp
# TODO: Perhaps do `autoreconf -i` instead of `./bootstrap`?
./bootstrap
./configure  --prefix="${ESOPIPELINE}"
make
pushd metis/tests
make check
popd
make install
popd
popd

# Check installation with
# esorex --recipe-dir="${ESOPIPELINE}" --recipes
