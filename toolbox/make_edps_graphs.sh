#!/usr/bin/env bash

# https://betterdev.blog/minimal-safe-bash-script-template/
set -Eeuo pipefail
DIR_SCRIPT=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)
DIR_DOCS="${DIR_SCRIPT}/../docs/generated"

# Ensure the edps is started to prevent startup message.
edps

edps -w metis.metis_wkf --graph > "${DIR_DOCS}/metis_wkf.dot"
dot -Tsvg "${DIR_DOCS}/metis_wkf.dot" > "${DIR_DOCS}/metis_wkf.svg"
dot -Tpng "${DIR_DOCS}/metis_wkf.dot" > "${DIR_DOCS}/metis_wkf.png"

edps -w metis.metis_wkf --detailed-graph > "${DIR_DOCS}/metis_wkf.detailed.dot"
dot -Tsvg "${DIR_DOCS}/metis_wkf.detailed.dot" > "${DIR_DOCS}/metis_wkf.detailed.svg"
dot -Tpng "${DIR_DOCS}/metis_wkf.detailed.dot" > "${DIR_DOCS}/metis_wkf.detailed.png"

edps -w metis.metis_wkf --assocmap > "${DIR_DOCS}/metis_wkf.association_map.md"
