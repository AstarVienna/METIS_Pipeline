from edps import QC1_CALIB, CALCHECKER
from edps import task, ReportInput, subworkflow

from .aspresso_datasources import *
from .aspresso_job_processing import *


@subworkflow("detector_properties", "")
def detector_properties():
    # --- Measure detector linearity
    detmon = (task('detmon')
              .with_recipe('detmon_opt_lg_mr')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
              .with_main_input(raw_detlin_on)
              .with_associated_input(raw_detlin_off, min_ret=10, max_ret=20)
              .with_job_processing(select_detmon_parameters)
              .with_meta_targets([QC1_CALIB])
              .build())

    # --- Process bias raw calibrations
    bias = (task('bias')
            .with_recipe('espdr_mbias')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_master_bias', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png')
            .with_main_input(raw_bias)
            .with_associated_input(ccd_geom)
            .with_associated_input(inst_config)
            .with_meta_targets([QC1_CALIB])
            .build())

    # --- Process dark  raw calibrations
    dark = (task('dark')
            .with_recipe('espdr_mdark')
            .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS, driver='png')
            .with_report('aspresso_master_dark', ReportInput.RECIPE_INPUTS_OUTPUTS, driver='png')
            .with_main_input(raw_dark)
            .with_associated_input(bias, [MASTER_BIAS_RES])
            .with_associated_input(ccd_geom)
            .with_associated_input(inst_config)
            .with_meta_targets([QC1_CALIB, CALCHECKER])
            .build())

    return bias, dark, detmon
