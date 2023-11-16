# Docker image for the METIS Pipeline.
# Also contains some development tools.
# Build with
#   docker build -t metispipeline .
# run from the root of this repository:
#   docker run -ti --net=host --env="DISPLAY" --volume="$HOME/.Xauthority:/home/metis/.Xauthority:rw" --mount type=bind,source="$(pwd)",target=/home/metis/METIS_Pipeline metispipeline

# Fedora is one of ESO's officially supported platforms.
FROM fedora:37

LABEL authors="Hugo"

# User as prescribed in
# https://mybinder.readthedocs.io/en/latest/tutorials/dockerfile.html
ARG NB_USER=metis
ARG NB_UID=1000
ENV USER ${NB_USER}
ENV NB_UID ${NB_UID}
ENV HOME /home/${NB_USER}

RUN adduser \
    --uid ${NB_UID} \
    ${NB_USER}

# Copy over the repository.
COPY . ${HOME}/METIS_Pipeline

# Install dependencies as root.
USER root
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_dependencies_fedora.sh
RUN chown -R ${NB_UID} ${HOME}

# Install the pipeline as a normal user.
USER ${NB_USER}
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/create_config.sh
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_metisp.sh
