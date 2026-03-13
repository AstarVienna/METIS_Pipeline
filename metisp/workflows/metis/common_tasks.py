from edps import task
from .metis_datasources import *
# dark_raw, persistence_map, detlin_raw, wcu_off_raw
from .metis_task_functions import which_detector, is_LM, is_N, is_IFU

lingain_task = (task('metis_lingain')
                .with_recipe("metis_det_lingain")
                .with_main_input(detlin_raw)
                .with_associated_input(wcu_off_raw, max_ret=100)  # otherwise only 1 is associated: default max_ret=1
                .build())

dark_task = (task('metis_img_dark')
             .with_main_input(dark_raw)
             .with_associated_input(lingain_task, [GAIN_MAP_2RG, LINEARITY_2RG, BADPIX_MAP_2RG], condition=is_LM)
             .with_associated_input(lingain_task, [GAIN_MAP_GEO, LINEARITY_GEO, BADPIX_MAP_GEO], condition=is_N)
             .with_associated_input(lingain_task, [GAIN_MAP_IFU, LINEARITY_IFU, BADPIX_MAP_IFU], condition=is_IFU)
             .with_dynamic_parameter("which_detector", which_detector)
             .with_associated_input(persistence_map)
             .with_recipe("metis_det_dark")
             .build())
