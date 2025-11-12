
# METIS Pipeline
The METIS pipeline is the data reduction software for the Mid-infrared E-ELT Imager and Spectrograph (METIS). 

## Manual Installation

Instructions for installation via docker containers look here [README.md](../toolbox/README.md)

For basic instructions on how to run workflows and recipes, go [here](#running-the-software)

## Install dependencies

The instructions below assume Ubuntu 24.04, but can easily be adapted for other systems.
```
apt-get install -y \
	wget gcc  automake autogen libtool gsl-bin libgsl-dev \
	libfftw3-bin libfftw3-dev fftw-dev \
	curl bzip2 less subversion git cppcheck lcov valgrind \
	zlib1g zlib1g-dev \
	liberfa1 liberfa-dev \
	libcurl4-openssl-dev libcurl4 \
	tmux ripgrep file \
	libcfitsio-bin libcfitsio-dev \
	wcslib-dev wcslib-tools \
	libcpl-dev \
	python3-astropy python3-matplotlib python3-numpy \
	perl cmake \
	graphviz meld \
	python3-pip python3-full \
	python3-jupyter-core python3-jupyter-client python3-notebook
```

## Create a Python environment
Use your favourite tool to create a Python environment, e.g. venv, conda, etc 
Python 3.12 is recommended. Then start the environment.

An example using virtualenv:
```
python -m venv metis_pip
source metis_pip/bin/activate
```

An alternative example using conda:
```
conda create -n metis python==3.12 poetry
conda activate metis
```

## Clone the METIS pipeline
```
git clone https://github.com/AstarVienna/METIS_Pipeline.git
```
and set the following environment variables:
```
export PYTHONPATH="$(pwd)/METIS_Pipeline/metisp/pymetis/src/"
export PYCPL_RECIPE_DIR="$(pwd)/METIS_Pipeline/metisp/pyrecipes/"
export PYESOREX_PLUGIN_DIR="$PYCPL_RECIPE_DIR"
```

## Install PyEsoRex, PyCPL and EDPS

Set the PYESOREX_PLUGIN_DIR environment variable: 
The PYESOREX_PLUGIN_DIR environment variable needs to be pointing to the recipes already before you install pyesorex and edps. See above for the export statement.

Install pyesorex and the EDPS in the Python environment.
```
pip install --extra-index-url \
    https://ftp.eso.org/pub/dfs/pipelines/libraries \
    pycpl pyesorex edps adari_core
```

Pyesorex and the EDPS need to be configured to use the METIS Pipeline. The easiest way to do this is by copying the provided configuration files. 

–WARNING–: 
Backup any existing EDPS configuration before proceeding.

Option 1 (beginner)
The simple way to configure the system environment to run EDPS is to run the following bash script from the toolbox:
```
./METIS_Pipeline/toolbox/create_config.sh
```
Option 2 (expert)
The hard way gives you more control about where the EDPS configuration is kept:
```
mkdir -p "/tmp/EDPS_data"
mkdir -p "${HOME}/.edps"
cp -avi METIS_Pipeline/toolbox/config/DOTedps/* "${HOME}/.edps"
```
Note that the above sets these parameters
```
base_dir=/tmp/EDPS_data
workflow_dir=.
```
since the absolute paths are not known.

For manual configuration, change these parameters in ~./edps/application.properties :
```
esorex_path=pyesorex
workflow_dir=/absolute/path/to/METIS_Pipeline/metisp/workflows
breakpoints_url=
```
Where the absolute path to the METIS workflow directory must be given. Setting the breakpoints_url is optional, but required to use the pipeline offline.


# Running the software

## Checking with PyESOREX

Firstly, run pyesorex.  We will see all avaliable receipes if there is not problem.

```
$ pyesorex --recipes
[ INFO  ] pyesorex: This is PyEsoRex, version 1.0.0.

     ***** ESO Recipe Execution Tool, Python version 1.0.0 *****

List of available recipes:

  metis_det_dark        : Create master dark
  metis_abstract_base   : Abstract-like base class for METIS recipes
  metis_det_lingain     : Measure detector non-linearity and gain
  metis_ifu_calibrate   : Calibrate IFU science data
  metis_ifu_reduce      : Reduce raw science exposures of the IFU.
  metis_ifu_telluric    : Derive telluric absorption correction and optionally flux calibration
  metis_lm_basic_reduction: Basic science image data processing
  metis_lm_img_flat     : Create master flat for L/M band detectors
  metis_n_img_flat      : Create master flat for N band detectors
```


## Use with EDPS
Before starting to use this pipeline with EDPS, make sure you have read the documentation for EDPS.  A good starting point is [here](https://www.eso.org/sci/software/edps.html).

To be safe, this command clears out all the cached data, logs, products, etc. This erases the whole EDPS history, and is therefore most useful when developing / debugging and shouldn't be done in regular operation. 
```
edps -shutdown ; rm -rf edps.log ;rm -rf pyesorex.log ; rm -rf $HOME/EDPS_data/* /tmp/EDPS_DATA/*
```

List all avaliable data files
```
 edps -w metis.metis_wkf -i $SOF_DATA -c
```

List all avaliable processing tasks
```
 edps -w metis.metis_wkf -i $SOF_DATA -lt
```

Running one specific recipe
```
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_detlin
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_lm_img_flat
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_lm_img_basic_reduce
 
```

Running Meta-target
```
 edps -w metis.metis_wkf -i $SOF_DATA -m science 
```

Getting reports in a better way
```
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -od
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -og
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -f
```

Making plots
```
edps -w metis.metis_lm_img_wkf -i $SOF_DATA -g > test.dot
dot -T png test.dot > mygraph.png
```
The generated plotting code can be plotted using an online tool as well
[GraphvizOnline](https://dreampuf.github.io/GraphvizOnline/)


## Note for developers
When you're using the Python Debugger (pdb) and an error occurs, pdb will automatically enter post-mortem debugging mode, allowing you to inspect the state of the program at the point where the error occurred. Here's how you can find out where the error happened:
```
import pdb ; pdb.set_trace()
```
