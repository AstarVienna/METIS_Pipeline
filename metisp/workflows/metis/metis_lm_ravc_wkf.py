from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, subworkflow, qc1calib, match_rules, FilterMode, calchecker
from .metis_datasources import *
from . import metis_keywords as metis_kwd
#from .metis_lm_img_wkf import lm_img_lingain_task, lm_img_dark_task, lm_img_flat_task, lm_img_distortion_task, lm_img_basic_reduce_sci_task, lm_img_basic_reduce_sci_task, lm_img_basic_reduce_sky_task, lm_img_basic_reduce_std_task, lm_img_background_sci_task, lm_img_basic_reduce_std_task, lm_img_standard_flux_task, lm_img_calib_task

from .metis_lm_img_wkf import *
lm_ravc_post_task1 = (task('lm_ravc_post1')
             .with_recipe('metis_img_adi_cgrph')
             .with_main_input(lm_img_calib_task)
             .with_meta_targets([SCIENCE])
             .build())
# QC1
