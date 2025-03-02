from edps import task, SCIENCE
from .metis_datasources import *

# --- Processing tasks ---


#TODO Make these prefer already created master calibrations
lingain_n_task = (task("metis_n_lingain")
                  .with_recipe("metis_det_lingain")
                  .with_main_input(detlin_n_raw)
                  .build())

distortion_n_task = (task("metis_n_distortion")
                     .with_recipe("metis_n_img_distortion")
                     .with_main_input(distortion_n_raw)
                     .build())

dark_n_task = (task("metis_n_dark")
               .with_recipe("metis_det_dark")
               .with_main_input(dark_n_raw)
               .build())

#TODO Find out if you can have multiple classification rules in one Data Source
flat_n_task = (task("metis_n_flat")
               .with_recipe("metis_n_img_flat")
               .with_main_input(flat_twi_n_raw)
               .with_associated_input(flat_lamp_n_raw)
               .build())

chopnod_n_task = (task("metis_n_chopnod")
                  .with_recipe("metis_n_img_chopnod")
                  .with_main_input(sci_n_raw)
                  .build())

std_process_n_task = (task("metis_n_std_process")
                      .with_recipe("metis_n_img_std_process")
                      .with_main_input(std_n_bkg_sub)
                      .build())

sci_calib_n_task = (task("metis_n_img_calibrate")
                    .with_recipe("metis_n_img_calibrate")
                    .with_main_input(sci_n_bkg_sub)
                    .build())

restore_n_task = (task("metis_n_restore")
                  .with_recipe("metis_n_img_restore")
                  .with_main_input(sci_n_calib)
                  .build())

#TODO add associated inputs