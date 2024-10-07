# METIS Pipeline
METIS pipe line is the data reduction software for ELT early science instrument of Mid-IR imager and spectrograph.  

## Path Setting

Including the following two line in your .bashrc

```
export METIS_SOFTPATH='<path_of_METIS_pipeline>' 
export PYTHONPATH=$METIS_SOFTPATH
export SOF_DATA='<path_to_data>'
export PYCPL_RECIPE_DIR='<path_of_METIS_pipeline>/prototypes/recipes/'
```
## Checking with PyESOREX

Firstly, run pyesorex.  We will see all avaliable receipes if there is not problem.

```
$ pyesorex --recipes
[ INFO  ] pyesorex: This is PyEsoRex, version 1.0.0.

     ***** ESO Recipe Execution Tool, Python version 1.0.0 *****

List of available recipes:

  metis_det_lingain     : Measure detector non-linearity and gain
  metis_det_dark        : Create master dark
  metis_abstract_base   : Abstract-like base class for METIS recipes
  metis_lm_basic_reduction: Basic science image data processing
  metis_n_img_flat      : Create master flat for N band detectors
  basic_science         : Basic science image data processing
  metis_lm_img_flat     : Create master flat for L/M band detectors
```


## Use with EDPS
Before starting to use this pipeline with EDPS, make sure you have read the document of EDPS.  You may also 
find some useful information [here](https://it.overleaf.com/project/65c1ef845dddcc9a7247e46c)

Remember to define the workflow path in .edps/application.properties.

```
workflow_dir=/home/chyan/METIS_Pipeline/prototypes/
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
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t basic_reduction

```

Getting report in a better way
```
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -od
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -og
 edps -w metis.metis_lm_img_wkf -i $SOF_DATA -t metis_det_dark -f
```


Making plots
```
edps -w metis.metis_lm_img_wkf -i /home/chyan/METIS_Simulations/ESO/output -g > test.dot
dot -T png test.dot > mygraph.png
```

## Note for developers
When you're using the Python Debugger (pdb) and an error occurs, pdb will automatically enter post-mortem debugging mode, allowing you to inspect the state of the program at the point where the error occurred. Here's how you can find out where the error happened:
```
import pdb ; pdb.set_trace()
```