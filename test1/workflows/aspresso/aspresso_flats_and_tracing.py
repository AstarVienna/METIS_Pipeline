from edps import QC1_CALIB, CALCHECKER
from edps import task, ReportInput, subworkflow

from .aspresso_datasources import *


@subworkflow("flats_and_tracing", "")
def flats_and_tracing(dark):
    # ---Process led flat fields
    led_ff = (task('led_ff')
              .with_recipe('espdr_led_ff')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
              .with_main_input(raw_led_ff)
              .with_associated_input(dark, [HOT_PIXEL_MASK])
              .with_associated_input(ccd_geom)
              .with_associated_input(inst_config)
              .with_associated_input(led_ff_gain_windows)
              .with_meta_targets([QC1_CALIB, CALCHECKER])
              .build())

    # --- Define order
    orderdef = (task('orderdef')
                .with_recipe('espdr_orderdef')
                .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
                .with_report('aspresso_orderdef_flatfield', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png')
                .with_main_input(raw_orderdef)
                .with_associated_input(dark, [HOT_PIXEL_MASK])
                .with_associated_input(led_ff, [BAD_PIXEL_MASK])
                .with_associated_input(ccd_geom)
                .with_associated_input(inst_config)
                .with_meta_targets([QC1_CALIB])
                .build())

    # --- Process raw flat calibrations
    flat = (task('flat')
            .with_recipe('espdr_mflat')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_echelle_flatfield', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png')
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
            .with_meta_targets([QC1_CALIB])
            .build())

    return led_ff, orderdef, flat
