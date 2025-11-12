
## Quick start

### Install system-wide prerequisites
For systems that use `apt`:
```
apt-get install -y \
	wget gcc automake autogen libtool gsl-bin libgsl-dev \
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

### Clone the repository
```
git clone https://github.com/AstarVienna/METIS_Pipeline.git
```

### Set up the virtual environment
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

Or with `pipenv`:
```
pipenv shell
```

Optionally, you may install `pytest` to be able to run automated tests, with
```
pip install pytest
```
or for `pipenv`
```
pipenv install --dev pytest
```

### Set up environment variables
For some reason the `PYESOREX_PLUGIN_DIR` environment variable needs to point
to the recipes, even **before** you install pyesorex and edps:

```
export PYTHONPATH="$(pwd)/METIS_Pipeline/metisp/pymetis/src/"
export PYCPL_RECIPE_DIR="$(pwd)/METIS_Pipeline/metisp/pyrecipes/"
export PYESOREX_PLUGIN_DIR="$PYCPL_RECIPE_DIR"
```

Optionally, you may put the `export`s into the `.rc` file of the shell of your choice.

The pipeline also requires file locations to be set:
```
export SOF_DATA="$(pwd)/METIS_Pipeline_Test_Data/small202402/outputSmall/"
export SOF_DIR="$(pwd)/METIS_Pipeline_Test_Data/small202402/sofFiles/"
export PYESOREX_OUTPUT_DIR="/tmp/"
```

### Install external ESO prerequisites
Probably the easiest way is to activate the virtual environment and then run 
```
pip install --extra-index-url \
    https://ftp.eso.org/pub/dfs/pipelines/libraries \
    pycpl pyesorex edps adari_core
```

### Run `pytest`
If everything is configured properly, you should be able to run `pytest` from within `.../metisp/pymetis`.
If EDPS has not yet been set up, `pytest -m "not edps"` deselects the related tests
(which also take much longer than the rest).
