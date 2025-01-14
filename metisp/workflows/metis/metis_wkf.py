from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, ReportInput, subworkflow

from .metis_lm_img_wkf import basic_reduction

@subworkflow("lm_img", "")
def lm_img():
    basic_reduction = (task('metis_lm_img_basic_reduce')
            .with_recipe('metis_lm_img_basic_reduce')
            .with_main_input(lm_raw_science)
            .with_associated_input(lingain_task)
            .with_associated_input(dark_task)
            .with_associated_input(flat_task)
            .with_meta_targets([QC1_CALIB])
            .build())
