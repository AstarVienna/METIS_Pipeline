# METIS Pipeline
METIS pipe line is the data reduction software for ELT early science instrument of Mid-IR imager and spectrograph.  

## Installation

```
cd metisp
./bootstrap
./configure --prefix=/tmp/wherever
make
make python
make install
make install-python
```


## Path Setting

Including the following two line in your .bashrc

```
export METIS_SOFTPATH='<path_of_METIS_pipeline>' 
export PYTHONPATH=$METIS_SOFTPATH
export SOF_DATA='<path_to_data>'
export PYCPL_RECIPE_DIR='<path_of_METIS_pipeline>/metisp/pymetis/src/pymetis/recipes/'
```
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
Before starting to use this pipeline with EDPS, make sure you have read the document of EDPS.  You may also 
find some useful information [here](https://it.overleaf.com/project/65c1ef845dddcc9a7247e46c)

Remember to define the workflow path in .edps/application.properties.

```
workflow_dir='<Parent path>'/METIS_Pipeline/metisp/workflows
```

To be safe, this command clear our all the cache data, log, product.
```
edps -shutdown ; rm -rf edps.log ;rm -rf pyesorex.log ; rm -rf EDPS_data/*
```

Listing all avaliable data files
```
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -c
```


Listing all avaliable processing tasks
```
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -lt
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


Getting report in a better way
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
The gerated plotting code can plot using online tool as well
[GraphvizOnline](https://dreampuf.github.io/GraphvizOnline/)


## Note for developers
When you're using the Python Debugger (pdb) and an error occurs, pdb will automatically enter post-mortem debugging mode, allowing you to inspect the state of the program at the point where the error occurred. Here's how you can find out where the error happened:
```
import pdb ; pdb.set_trace()
```