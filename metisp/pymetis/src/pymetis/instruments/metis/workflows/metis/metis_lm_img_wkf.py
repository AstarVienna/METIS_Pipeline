from edps import SCIENCE
from edps import task, ReportInput
from .metis_classification import (
    flux_tab_class,
    gain_map_h2rg_class,
    linearity_h2rg_class,
    lm_distortion_table_class,
    lm_sci_basic_reduced_class,
    lm_sci_bkg_subtracted_class,
    lm_sci_calibrated_class,
    lm_sky_basic_reduced_class,
    lm_std_basic_reduced_class,
    lm_std_bkg_subtracted_class,
    master_dark_2rg_class,
    master_img_flat_lamp_lm_class,
    master_img_flat_twilight_lm_class,
)
from .metis_datasources import (
    dark_2rg_raw,
    detlin_2rg_raw,
    fluxstd_catalog,
    lm_distortion_raw,
    lm_flat_lamp_raw,
    lm_flat_twilight_raw,
    lm_image_sci_raw,
    lm_image_sky_raw,
    lm_image_std_raw,
    lm_wcu_off_raw,
    persistence_map,
    pinhole_table,
)
from .metis_task_functions import setup_lingain_job

lm_img_lingain_task = (task('metis_lm_img_lingain')
                .with_main_input(detlin_2rg_raw)
                .with_recipe("metis_det_lingain")
                .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                .with_report("metis_det_lingain", ReportInput.RECIPE_INPUTS_OUTPUTS)
                .with_job_processing(setup_lingain_job)
                .build())

lm_img_dark_task = (task('metis_lm_img_dark')
            .with_main_input(dark_2rg_raw)
            .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
            .with_associated_input(persistence_map)
            .with_recipe("metis_det_dark")
            .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
            .with_report("metis_det_dark", ReportInput.RECIPE_INPUTS_OUTPUTS)
            .build())

lm_img_flat_twilight_task = (task("metis_lm_img_flat_twilight")
             .with_main_input(lm_flat_twilight_raw)
             .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
             .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
             .with_associated_input(persistence_map)
             .with_recipe("metis_lm_img_flat_twilight")
             .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
             .with_report("metis_lm_img_flat_twilight", ReportInput.RECIPE_INPUTS_OUTPUTS)
             .build())

lm_img_flat_lamp_task = (task("metis_lm_img_flat_lamp")
             .with_main_input(lm_flat_lamp_raw)
             .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
             .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
             .with_associated_input(persistence_map)
             .with_associated_input(lm_wcu_off_raw)
             .with_recipe("metis_lm_img_flat_lamp")
             .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
             .with_report("metis_lm_img_flat_lamp", ReportInput.RECIPE_INPUTS_OUTPUTS)
             .build())

lm_img_distortion_task = (task('metis_lm_img_distortion')
                   .with_main_input(lm_distortion_raw)
                   .with_associated_input(lm_wcu_off_raw)
                   .with_associated_input(pinhole_table)
                   .with_associated_input(lm_img_flat_lamp_task, [master_img_flat_lamp_lm_class])
                   .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
                   .with_associated_input(persistence_map)
                   .with_recipe('metis_lm_img_distortion')
                   .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                   .with_report("metis_lm_img_distortion", ReportInput.RECIPE_INPUTS_OUTPUTS)
                   .build())

lm_img_basic_reduce_sci_task = (task('metis_lm_img_basic_reduce_sci')
                    .with_recipe('metis_lm_img_basic_reduce')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_basic_reduce", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_image_sci_raw)
                    .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
                    .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
                    .with_associated_input(lm_img_flat_twilight_task, [master_img_flat_twilight_lm_class])
                    .with_associated_input(persistence_map)
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_basic_reduce_sky_task = (task('metis_lm_img_basic_reduce_sky')
                    .with_recipe('metis_lm_img_basic_reduce')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_basic_reduce", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_image_sky_raw)
                    .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
                    .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
                    .with_associated_input(lm_img_flat_twilight_task, [master_img_flat_twilight_lm_class])
                    .with_associated_input(persistence_map)
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_basic_reduce_std_task = (task('metis_lm_img_basic_reduce_std')
                    .with_recipe('metis_lm_img_basic_reduce')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_basic_reduce", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_image_std_raw)
                    .with_associated_input(lm_img_lingain_task, [linearity_h2rg_class, gain_map_h2rg_class])
                    .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
                    .with_associated_input(lm_img_flat_twilight_task, [master_img_flat_twilight_lm_class])
                    .with_associated_input(persistence_map)
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_background_sci_task = (task('metis_lm_img_background_sci')
                    .with_recipe('metis_lm_img_background')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_background", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_img_basic_reduce_sci_task, [lm_sci_basic_reduced_class])
                    #.with_associated_input(lm_img_basic_reduce_sky_task, [lm_sky_basic_reduced_class])
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_background_std_task = (task('metis_lm_img_background_std')
                    .with_recipe('metis_lm_img_background')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_background", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_img_basic_reduce_std_task, [lm_std_basic_reduced_class])
                    #.with_associated_input(lm_img_basic_reduce_sky_task, [lm_sky_basic_reduced_class])
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_standard_flux_task = (task('metis_lm_img_standard_flux')
                    .with_recipe('metis_lm_img_std_process')
                    .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
                    .with_report("metis_lm_img_std_process", ReportInput.RECIPE_INPUTS_OUTPUTS)
                    .with_main_input(lm_img_background_std_task, [lm_std_bkg_subtracted_class])
                    .with_associated_input(fluxstd_catalog)
                    .with_meta_targets([SCIENCE])
                    .build())

lm_img_calib_task = (task('metis_lm_img_calib')
             .with_recipe('metis_lm_img_calibrate')
             .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
             .with_report("metis_lm_img_calibrate", ReportInput.RECIPE_INPUTS_OUTPUTS)
             .with_main_input(lm_img_background_sci_task, [lm_sci_bkg_subtracted_class])
             .with_associated_input(lm_img_standard_flux_task, [flux_tab_class])
             .with_associated_input(lm_img_distortion_task, [lm_distortion_table_class])
             .with_meta_targets([SCIENCE])
             .build())

lm_img_coadd_task = (task('metis_lm_img_coadd')
             .with_recipe('metis_lm_img_sci_postprocess')
             .with_report("metis_rawdisp", ReportInput.RECIPE_INPUTS)
             .with_report("metis_lm_img_sci_postprocess", ReportInput.RECIPE_INPUTS_OUTPUTS)
             .with_main_input(lm_img_calib_task, [lm_sci_calibrated_class])
             .with_meta_targets([SCIENCE])
             .build())
# QC1
