#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# Set environment variables.
source "${script_dir}/set_environment.sh"

# Enter root of this checkout.
pushd "${script_dir}/.."

cp -avi ./toolbox/config/DOTedps /home/metis/.edps
cp -avi ./toolbox/config/DOTesorex /home/metis/.esorex
mkdir /home/metis/EDPS_data
