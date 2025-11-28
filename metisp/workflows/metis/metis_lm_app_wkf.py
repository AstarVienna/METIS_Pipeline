from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd

from .metis_lm_img_wkf import *
lm_adi_app_post_task1 = (task('lm_app_post')
             .with_recipe('metis_lm_adi_app')
             .with_main_input(lm_img_calib_task)
             .with_meta_targets([SCIENCE])
             .build())
# QC1
