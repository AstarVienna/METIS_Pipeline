from edps import QC1_CALIB, CALCHECKER
from edps import task, ReportInput, subworkflow

from .aspresso_datasources import *
from .aspresso_job_processing import *


@subworkflow("pre_reduction", "")
def pre_reduction():
    detmon = (task('detmon')
              .with_meta_targets([QC1_CALIB])
              .with_job_processing(select_detmon_parameters)
              .with_main_input(raw_detlin_on)
              .with_associated_input(raw_detlin_off, min_ret=10, max_ret=20)
              .with_recipe('detmon_opt_lg_mr')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png').build())

    bias = (task('bias')
            .with_meta_targets([QC1_CALIB])
            .with_main_input(raw_bias)
            .with_associated_input(ccd_geom)
            .with_associated_input(inst_config)
            .with_recipe('espdr_mbias')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_master_bias', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png').build())

    dark = (task('dark')
            .with_meta_targets([QC1_CALIB, CALCHECKER])
            .with_main_input(raw_dark)
            .with_associated_input(bias, [MASTER_BIAS_RES])
            .with_associated_input(ccd_geom)
            .with_associated_input(inst_config)
            .with_recipe('espdr_mdark')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_master_dark', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png').build())

    led_ff = (task('led_ff')
              .with_meta_targets([QC1_CALIB, CALCHECKER])
              .with_main_input(raw_led_ff)
              .with_associated_input(dark, [HOT_PIXEL_MASK])
              .with_associated_input(ccd_geom)
              .with_associated_input(inst_config)
              .with_associated_input(led_ff_gain_windows)
              .with_recipe('espdr_led_ff')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png').build())

    orderdef = (task('orderdef')
                .with_meta_targets([QC1_CALIB])
                .with_main_input(raw_orderdef)
                .with_associated_input(dark, [HOT_PIXEL_MASK])
                .with_associated_input(led_ff, [BAD_PIXEL_MASK])
                .with_associated_input(ccd_geom)
                .with_associated_input(inst_config)
                .with_recipe('espdr_orderdef')
                .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
                .with_report('aspresso_orderdef_flatfield', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png').build())

    flat = (task('flat')
            .with_meta_targets([QC1_CALIB])
            .with_main_input(raw_flat)
            .with_associated_input(dark, [HOT_PIXEL_MASK])
            .with_associated_input(led_ff, [BAD_PIXEL_MASK])
            .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
            .with_associated_input(static_wave_matrix_a, min_ret=1, max_ret=1)
            .with_associated_input(static_wave_matrix_b, min_ret=1, max_ret=1)
            .with_associated_input(static_dll_matrix_a, min_ret=1, max_ret=1)
            .with_associated_input(static_dll_matrix_b, min_ret=1, max_ret=1)
            .with_associated_input(ccd_geom)
            .with_associated_input(inst_config)
            .with_input_filter(ccd_geom_class, inst_config_class, static_dll_matrix_a_class, static_wave_matrix_b_class,
                               static_dll_matrix_b_class, static_wave_matrix_a_class,
                               HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B)
            .with_recipe('espdr_mflat')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_echelle_flatfield', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png').build())

    return bias, dark, led_ff, orderdef, flat, detmon
