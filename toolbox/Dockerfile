# Docker image for the METIS Pipeline.
# Also contains some development tools.
# Build with
#   docker build -t metispipeline .
# run from the root of this repository:
#   docker run -ti --net=host --env="DISPLAY" --volume="$HOME/.Xauthority:/home/metis/.Xauthority:rw" --mount type=bind,source="$(pwd)",target=/home/metis/METIS_Pipeline metispipeline

# Fedora is one of ESO's officially supported platforms.
FROM fedora:39

LABEL authors="Hugo Buddelmeijer, Gilles Otten"
MAINTAINER "ELT/METIS Team"

# User as prescribed in
# https://mybinder.readthedocs.io/en/latest/tutorials/dockerfile.html
ARG NB_USER=metis
ARG NB_UID=1000
ENV USER ${NB_USER}
ENV NB_UID ${NB_UID}
ENV HOME /home/${NB_USER}

RUN useradd \
    --create-home \
    --shell /bin/bash \
    ${NB_USER}

RUN dnf install -y git

# Clone the repositories.
USER ${NB_USER}
WORKDIR /home/metis
RUN git clone https://github.com/AstarVienna/METIS_Pipeline.git
RUN git clone https://github.com/AstarVienna/METIS_Pipeline_Test_Data.git
RUN git clone https://github.com/AstarVienna/METIS_Simulations.git

# Install dependencies as root.
USER root
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_dependencies_fedora.sh
#RUN chown -R ${NB_UID} ${HnOME}

# Install the pipeline as a normal user.
USER ${NB_USER}
#RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_edps.sh
RUN pip install --user --extra-index-url https://ftp.eso.org/pub/dfs/pipelines/libraries pycpl pyesorex edps adari_core;
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/create_config.sh
#RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_metisp.sh

WORKDIR /home/metis/METIS_Pipeline
ENV PYESOREX_PLUGIN_DIR "$HOME/METIS_Pipeline/metisp/pymetis/src/pymetis/recipes"
ENV PYCPL_RECIPE_DIR "$HOME/METIS_Pipeline/metisp/pyrecipes/"
ENV PYTHONPATH "$HOME/METIS_Pipeline/metisp/pymetis/src/"
ENV SOF_DATA "$HOME/METIS_Pipeline_Test_Data/metis_sim_small_1/data"
ENV SOF_DIR "$HOME/METIS_Pipeline_Test_Data/metis_sim_small_1/sof"
