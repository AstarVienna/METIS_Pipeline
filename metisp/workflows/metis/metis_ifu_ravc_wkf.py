from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd
from .metis_ifu_wkf import *

ifu_adi_cgrph = (task('metis_ifu_adi_cgrph')
             .with_recipe('metis_ifu_adi_cgrph')
             .with_main_input(ifu_calibrate_task)
             .with_meta_targets([SCIENCE])
             .build())

