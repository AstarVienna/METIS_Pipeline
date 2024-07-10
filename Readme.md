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


Making plots
```
edps -w metis.metis_dark_wkf -i /home/chyan/METIS_Simulations/ESO/output -g > test.dot
dot -T png test.dot > mygraph.png
```