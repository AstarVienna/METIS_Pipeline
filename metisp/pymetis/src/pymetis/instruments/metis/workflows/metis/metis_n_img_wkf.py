from edps import SCIENCE
from edps import task
from .metis_classification import (
    flux_tab_class,
    gain_map_geo_class,
    linearity_geo_class,
    master_dark_geo_class,
    master_img_flat_lamp_n_class,
    master_img_flat_twilight_n_class,
    n_distortion_table_class,
    n_sci_bkg_subtracted_class,
    n_sci_calibrated_class,
    n_std_bkg_subtracted_class,
)
from .metis_datasources import (
    dark_geo_raw,
    detlin_geo_raw,
    fluxstd_catalog,
    n_distortion_raw,
    n_flat_lamp_raw,
    n_image_sci_raw,
    n_image_std_raw,
    n_wcu_off_raw,
    persistence_map,
    pinhole_table,
)
from .metis_task_functions import setup_lingain_job

n_img_lingain_task = (task('metis_n_img_lingain')
                .with_recipe("metis_det_lingain")
                .with_main_input(detlin_geo_raw)
                .with_job_processing(setup_lingain_job)
                .build())

n_img_dark_task = (task('metis_n_img_dark')
            .with_main_input(dark_geo_raw)
            .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
            .with_associated_input(persistence_map)
            .with_recipe("metis_det_dark")
            .build())

n_img_flat_lamp_task = (task("metis_n_img_flat_lamp")
             .with_main_input(n_flat_lamp_raw)
             .with_associated_input(n_img_dark_task, [master_dark_geo_class])
             .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
             .with_associated_input(persistence_map)
             .with_associated_input(n_wcu_off_raw)
             .with_recipe("metis_n_img_flat_lamp")
             .build())

n_img_flat_twilight_task = (task("metis_n_img_flat_twilight")
             .with_main_input(n_flat_lamp_raw)
             .with_associated_input(n_img_dark_task, [master_dark_geo_class])
             .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
             .with_associated_input(persistence_map)
             .with_recipe("metis_n_img_flat_twilight")
             .build())

n_img_distortion_task = (task('metis_n_img_distortion')
                   .with_main_input(n_distortion_raw)
                   .with_associated_input(n_wcu_off_raw)
                   .with_associated_input(pinhole_table)
                   .with_associated_input(n_img_flat_lamp_task, [master_img_flat_lamp_n_class])
                   .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
                   .with_associated_input(persistence_map)
                   .with_recipe('metis_n_img_distortion')
                   .build())

n_img_chopnod_sci_task = (task('metis_n_img_chopnod_sci')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_image_sci_raw)
                    .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
                    .with_associated_input(n_img_dark_task, [master_dark_geo_class])
                    .with_associated_input(n_img_flat_twilight_task, [master_img_flat_twilight_n_class])
                    .with_associated_input(persistence_map)
                    .with_meta_targets([SCIENCE])
                    .build())

n_img_chopnod_std_task = (task('metis_n_img_chopnod_std')
                    .with_recipe('metis_n_img_chopnod')
                    .with_main_input(n_image_std_raw)
                    .with_associated_input(n_img_lingain_task, [linearity_geo_class, gain_map_geo_class])
                    .with_associated_input(n_img_dark_task, [master_dark_geo_class])
                    .with_associated_input(n_img_flat_twilight_task, [master_img_flat_twilight_n_class])
                    .with_associated_input(persistence_map)
                    .with_meta_targets([SCIENCE])
                    .build())

n_img_standard_flux_task = (task('metis_n_img_standard_flux')
                    .with_recipe('metis_n_img_std_process')
                    .with_main_input(n_img_chopnod_std_task, [n_std_bkg_subtracted_class])
                    .with_associated_input(fluxstd_catalog)
                    .with_meta_targets([SCIENCE])
                    .build())

n_img_calib_task = (task('metis_n_img_calib')
             .with_recipe('metis_n_img_calibrate')
             .with_main_input(n_img_chopnod_sci_task, [n_sci_bkg_subtracted_class])
             .with_associated_input(n_img_standard_flux_task, [flux_tab_class])
             .with_associated_input(n_img_distortion_task, [n_distortion_table_class])
             .with_meta_targets([SCIENCE])
             .build())

n_img_restore_task = (task('metis_n_img_restore')
             .with_recipe('metis_n_img_restore')
             .with_main_input(n_img_calib_task, [n_sci_calibrated_class])
             .with_meta_targets([SCIENCE])
             .build())
# QC1
