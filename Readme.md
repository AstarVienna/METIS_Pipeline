# METIS Pipeline


## Path Setting

Including the following two line in your .bashrc

```
export METIS_SOFTPATH='<path_of_METIS_pipeline>' 
export PYTHONPATH=$METIS_SOFTPATH
export SOF_DATA='<path_to_data>'
export PYCPL_RECIPE_DIR='<path_of_METIS_pipeline>/prototypes/recipes/'
```

## Use with EDPS

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