from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd


dark_task = (task('metis_det_dark')
            .with_main_input(raw_dark)
            .with_recipe("metis_det_dark")
            .build())

lingain_task = (task('metis_det_detlin')
            .with_main_input(detlin_raw)
            .with_associated_input(dark_task)
            .with_recipe("metis_det_lingain")
            .build())

flat_task = (task("metis_lm_img_flat")
            .with_main_input(lm_lamp_flat)
            .with_associated_input(dark_task)
            .with_recipe("metis_lm_img_flat")
            .build())

basic_reduction = (task('metis_lm_img_basic_reduce')
                    .with_recipe('metis_lm_img_basic_reduce')
                    .with_main_input(lm_raw_science)
                    .with_associated_input(lingain_task)
                    .with_associated_input(dark_task)
                    .with_associated_input(flat_task)
                    .with_meta_targets([SCIENCE])
                    .build())

