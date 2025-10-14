from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd

# TODO: Make Unique names for dark and detlin recipe

dark_geo_task = (task('metis_det_dark')
            .with_main_input(raw_geo_dark)
            .with_recipe("metis_det_dark")
            .build())

lingain_geo_task = (task('metis_det_detlin')
                .with_recipe("metis_det_lingain")
                .with_main_input(detlin_geo_raw)
                .with_associated_input(dark_geo_task)
                .with_associated_input(n_wcu_off_raw)
                .build())

n_flat_task = (task("metis_n_img_flat")
             .with_main_input(n_lamp_flat)
             .with_associated_input(dark_geo_task)
             .with_associated_input(lingain_geo_task)
             .with_recipe("metis_n_img_flat")
             .build())

n_distortion_task = (task('metis_n_img_cal_distortion')
                   .with_main_input(n_distortion_raw)
                   .with_associated_input(n_wcu_off_raw)
                   .with_associated_input(pinehole_tab)
                   .with_associated_input(lingain_geo_task)
                   .with_recipe('metis_n_img_distortion')
                   .build())

n_img_chopnod_sci_task = (task('metis_n_img_chopnod_sci')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_raw_science)
                    .with_associated_input(lingain_geo_task)
                    .with_associated_input(dark_geo_task)
                    .with_associated_input(n_flat_task)
                    .with_meta_targets([SCIENCE])
                    .build())

n_img_chopnod_std_task = (task('metis_n_img_chopnod_std')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_raw_std)
                    .with_associated_input(lingain_geo_task)
                    .with_associated_input(dark_geo_task)
                    .with_associated_input(n_flat_task)
                    .with_meta_targets([SCIENCE])
                    .build())

n_standard_flux_task = (task('metis_n_img_standard_flux')
                    .with_recipe('metis_n_img_std_process')
                    .with_main_input(n_img_chopnod_std_task)
                    .with_associated_input(fluxstd_catalog)
                    .with_meta_targets([SCIENCE])
                    .build())

n_img_calib = (task('metis_n_img_calib')
             .with_recipe('metis_n_img_calibrate')
             .with_main_input(n_img_chopnod_sci_task)
             .with_associated_input(n_standard_flux_task)
             .with_associated_input(n_distortion_task)
             .with_meta_targets([SCIENCE])
             .build())
             
n_img_restore = (task('metis_n_img_restore')
             .with_recipe('metis_n_img_restore')
             .with_main_input(n_img_calib)
             .with_meta_targets([SCIENCE])
             .build())
# QC1
