from edps import task, SCIENCE
from .metis_datasources import *

# --- Processing tasks ---


#TODO Make these prefer already created master calibrations
lingain_n_task = (task("metis_n_lingain")
                  .with_recipe("metis_det_lingain")
                  .with_main_input(detlin_n_raw)
                  .with_associated_input(bad_pix_n_img_calib, min_ret=0)
                  .build())

distortion_n_task = (task("metis_n_distortion")
                     .with_recipe("metis_n_img_distortion")
                     .with_main_input(distortion_n_raw)
                     .with_associated_input(bad_pix_n_img_calib, min_ret=0)
                     .with_associated_input(lingain_n_task)
                     .with_associated_input(calib_persistence)
                     .with_associated_input(pinehole_tab)
                     .build())

dark_n_task = (task("metis_n_dark")
               .with_recipe("metis_det_dark")
               .with_main_input(dark_n_raw)
               .with_associated_input(bad_pix_n_img_calib, min_ret=0)
               .with_associated_input(lingain_n_task)
               .with_associated_input(calib_persistence)
               .build())

#TODO Find out if you can have multiple classification rules in one Data Source
flat_n_task = (task("metis_n_flat")
               .with_recipe("metis_n_img_flat")
               .with_main_input(flat_twi_n_raw)
               .with_associated_input(bad_pix_n_img_calib, min_ret=0)
               .with_associated_input(flat_lamp_n_raw)
               .with_associated_input(lingain_n_task)
               .with_associated_input(calib_persistence)
               .with_associated_input(dark_n_task)
               .build())

chopnod_n_task = (task("metis_n_chopnod")
                  .with_recipe("metis_n_img_chopnod")
                  .with_main_input(sci_n_raw)
                  .with_associated_input(bad_pix_n_img_calib, min_ret=0)
                  .with_associated_input(lingain_n_task)
                  .with_associated_input(calib_persistence)
                  .with_associated_input(dark_n_task)
                  .with_associated_input(flat_n_task)
                  .build())

std_process_n_task = (task("metis_n_std_process")
                      .with_recipe("metis_n_img_std_process")
                      .with_main_input(std_n_bkg_sub)
                      .with_associated_input(chopnod_n_task)
                      .with_associated_input(calib_flux_std)
                      .build())

sci_calib_n_task = (task("metis_n_img_calibrate")
                    .with_recipe("metis_n_img_calibrate")
                    .with_main_input(sci_n_bkg_sub)
                    .with_associated_input(chopnod_n_task)
                    .with_associated_input(std_process_n_task)
                    .with_associated_input(distortion_n_task)
                    .build())

restore_n_task = (task("metis_n_restore")
                  .with_recipe("metis_n_img_restore")
                  .with_main_input(sci_n_calib)
                  .with_associated_input(sci_calib_n_task)
                  .build())