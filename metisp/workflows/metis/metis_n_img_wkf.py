from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd

dark_geo_n_img_task = (task('metis_det_dark')
            .with_main_input(raw_geo_dark)
            .with_recipe("metis_det_dark")
            .build())

lingain_geo_n_img_task = (task('metis_det_detlin')
                .with_recipe("metis_det_lingain")
                .with_main_input(detlin_geo_raw)
                .with_associated_input(dark_geo_n_img_task)
                .with_associated_input(n_wcu_off_raw)
                .build())

flat_n_img_task = (task("metis_n_img_flat")
             .with_main_input(n_lamp_flat)
             .with_associated_input(dark_geo_n_img_task)
             .with_associated_input(lingain_geo_n_img_task)
             .with_recipe("metis_n_img_flat")
             .build())

distortion_n_img_task = (task('metis_n_img_cal_distortion')
                   .with_main_input(n_distortion_raw)
                   .with_associated_input(n_wcu_off_raw)
                   .with_associated_input(pinehole_tab)
                   .with_associated_input(lingain_geo_n_img_task)
                   .with_recipe('metis_n_img_distortion')
                   .build())

img_chopnod_sci_n_img_task = (task('metis_n_img_chopnod_sci')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_raw_science)
                    .with_associated_input(lingain_geo_n_img_task)
                    .with_associated_input(dark_geo_n_img_task)
                    .with_associated_input(flat_n_img_task)
                    .with_meta_targets([SCIENCE])
                    .build())

img_chopnod_std_n_img_task = (task('metis_n_img_chopnod_std')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_raw_std)
                    .with_associated_input(lingain_geo_n_img_task)
                    .with_associated_input(dark_geo_n_img_task)
                    .with_associated_input(flat_n_img_task)
                    .with_meta_targets([SCIENCE])
                    .build())

standard_flux_n_img_task = (task('metis_n_img_standard_flux')
                    .with_recipe('metis_n_img_std_process')
                    .with_main_input(img_chopnod_std_n_img_task)
                    .with_associated_input(fluxstd_catalog)
                    .with_meta_targets([SCIENCE])
                    .build())

calib_n_img_task = (task('metis_n_img_calib')
             .with_recipe('metis_n_img_calibrate')
             .with_main_input(img_chopnod_sci_n_img_task)
             .with_associated_input(standard_flux_n_img_task)
             .with_associated_input(distortion_n_img_task)
             .with_meta_targets([SCIENCE])
             .build())
             
restore_n_img_task = (task('metis_n_img_restore')
             .with_recipe('metis_n_img_restore')
             .with_main_input(calib_n_img_task)
             .with_meta_targets([SCIENCE])
             .build())
# QC1
