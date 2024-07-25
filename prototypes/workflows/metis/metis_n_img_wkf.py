"""EDPS workflow for METIS, mode IMG_N"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, classification_rule
from .metis_n_img_datasources import *

# --- Processing tasks ---
detlin_task = (task("metis_det_lingain")
               .with_recipe("metis_det_lingain")
               .with_main_input(detlin_raw)
               .with_meta_targets([QC0, QC1_CALIB])
               .build())

dark_task = (task("metis_det_dark")
             .with_recipe("metis_det_dark")
             .with_main_input(dark_raw)
             .with_meta_targets([QC0, QC1_CALIB])
             .build())

flat_lamp_task = (task("metis_n_flat_lamp")
                  .with_recipe("metis_n_img_flat")
                  .with_main_input(flat_lamp_raw)
                  .with_associated_input(detlin_task)
                  .with_meta_targets([QC0, QC1_CALIB])
                  .build())

flat_twilight_task = (task("metis_n_flat_twilight")
                      .with_recipe("metis_n_img_flat")
                      .with_main_input(flat_twilight_raw)
                      .with_associated_input(detlin_task)
                      .with_meta_targets([QC0, QC1_CALIB])
                      .build())

sci_chopnod_task = (task("chopnod_sci")
                    .with_recipe("metis_n_img_chopnod")
                    .with_main_input(sci_n_img_raw)
                    .with_associated_input(detlin_task)
                    .with_associated_input(flat_twilight_task)
                    .with_meta_targets([SCIENCE])
                    .build())

std_chopnod_task = (task("chopnod_std")
                    .with_recipe("metis_n_img_chopnod")
                    .with_main_input(std_n_img_raw)
                    .with_associated_input(detlin_task)
                    .with_associated_input(flat_twilight_task)
                    .with_meta_targets([QC0, QC1_CALIB])
                    .build())

std_process_task = (task("std_process")
                    .with_recipe("metis_n_img_std_process")
                    .with_main_input(std_chopnod_task)
                    .with_associated_input(fluxstd_cat)
                    .with_meta_targets([QC0, QC1_CALIB])
                    .build())

calibrate_task = (task("calibrate")
                  .with_recipe("metis_n_img_calibrate")
                  .with_main_input(sci_chopnod_task)
                  .with_associated_input(std_process_task)
                  .with_meta_targets([SCIENCE])
                  .build())

restore_task = (task("restore")
                .with_recipe("metis_n_img_restore")
                .with_main_input(calibrate_task)
                .with_meta_targets([SCIENCE])
                .build())
