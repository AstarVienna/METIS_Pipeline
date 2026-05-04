Installation
============

Prerequisites
-------------

The pipeline depends on ESO's Common Pipeline Library (CPL) and the associated Python
bindings.  These must be installed before ``pymetis``.

**System packages** (Ubuntu 24.04; adapt for other distributions)::

    sudo apt-get install -y \
        wget gcc automake autogen libtool \
        libgsl-dev libfftw3-dev libcurl4-openssl-dev \
        libcfitsio-dev wcslib-dev libcpl-dev \
        python3-pip python3-full

Python environment
------------------

Python 3.12 is recommended.  Create a dedicated virtual environment::

    python -m venv metis_env
    source metis_env/bin/activate

or with conda::

    conda create -n metis python=3.12
    conda activate metis

Clone the repository
--------------------

::

    git clone https://github.com/AstarVienna/METIS_Pipeline.git
    cd METIS_Pipeline

Set the required environment variables so that ``pyesorex`` can find the METIS recipes::

    export PYTHONPATH="$(pwd)/metisp/pymetis/src/"
    export PYCPL_RECIPE_DIR="$(pwd)/metisp/pyrecipes/"
    export PYESOREX_PLUGIN_DIR="$PYCPL_RECIPE_DIR"

.. important::

   ``PYESOREX_PLUGIN_DIR`` must be set **before** you install ``pyesorex`` so that
   it registers the plugin directory during installation.

Install Python dependencies
---------------------------

::

    pip install --extra-index-url \
        https://ftp.eso.org/pub/dfs/pipelines/libraries \
        pycpl pyesorex edps adari_core

Configure EDPS
--------------

The simplest approach is to run the provided configuration script::

    ./toolbox/create_config.sh

For manual configuration, copy the provided EDPS config files and adjust the
absolute paths in ``~/.edps/application.properties``::

    mkdir -p ~/.edps
    cp -av toolbox/config/DOTedps/* ~/.edps/

Then edit ``~/.edps/application.properties``::

    esorex_path=pyesorex
    workflow_dir=/absolute/path/to/METIS_Pipeline/metisp/workflows
    base_dir=/tmp/EDPS_data

Verify the installation
-----------------------

Check that ``pyesorex`` can discover the METIS recipes::

    pyesorex --recipes

You should see output like::

    List of available recipes:
      metis_det_dark        : Create master dark
      metis_det_lingain     : Measure detector non-linearity and gain
      metis_lm_basic_reduction: Basic science image data processing
      ...

Building the documentation
--------------------------

Install Sphinx and the required extensions::

    pip install -r metisp/pymetis/docs/requirements.txt

Then build the HTML documentation::

    cd metisp/pymetis/docs
    make html

Open ``_build/html/index.html`` in a browser.
