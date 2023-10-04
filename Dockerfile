# Docker image for the METIS Pipeline.
# Also contains some development tools.
# Build with
# docker build -t metispipeline .

# Fedora is one of ESO's officially supported platforms.
FROM fedora:latest

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

# A copy of the installation in toolbox/install_dependencies_fedora.sh
RUN dnf install -y \
    wget gcc gcc-g++ automake autogen libtool gsl gsl-devel fftw fftw-devel \
    curl bzip2 less svn git which dnf-plugins-core cppcheck lcov valgrind \
    erfa erfa-devel \
    libcurl-devel libcurl \
    tmux ripgrep file \
    cfitsio wcslib cpl esorex cfitsio-devel wcslib-devel cpl-devel


# Copy over the repository.
COPY . ${HOME}/METIS_Pipeline

# Install dependencies as root.
USER root
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_dependencies_fedora.sh
RUN chown -R ${NB_UID} ${HOME}

# Install the pipeline as a normal user.
USER ${NB_USER}
RUN bash -l ${HOME}/METIS_Pipeline/toolbox/install_metisp.sh
