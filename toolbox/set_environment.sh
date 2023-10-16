#!/usr/bin/env bash

# Set required environment variables
export TOINSTALLDIR="${TOINSTALLDIR:-${HOME}/download}"
export ESOPIPELINE="${ESOPIPELINE:-${HOME}/metis_pipeline}"
export CFITSIODIR="${ESOPIPELINE}"
export WCSDIR="${ESOPIPELINE}"
export CPLDIR="${ESOPIPELINE}"
export PATH=${ESOPIPELINE}/bin:${PATH}
