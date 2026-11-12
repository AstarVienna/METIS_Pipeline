from edps import SCIENCE
from edps import task
from .metis_classification import lm_sci_calibrated_class
from .metis_lm_img_wkf import lm_img_calib_task
# Re-export coaddition as an EDPS target.
from .metis_lm_img_wkf import lm_img_coadd_task as lm_img_coadd_task
#from .metis_lm_img_wkf import lm_img_lingain_task, lm_img_dark_task, lm_img_flat_twilight_task, lm_img_distortion_task, lm_img_basic_reduce_sci_task, lm_img_basic_reduce_sci_task, lm_img_basic_reduce_sky_task, lm_img_basic_reduce_std_task, lm_img_background_sci_task, lm_img_basic_reduce_std_task, lm_img_standard_flux_task, lm_img_calib_task

metis_lm_app_post_task = (task('metis_lm_app_post')
             .with_recipe('metis_img_adi_cgrph')
             .with_main_input(lm_img_calib_task, [lm_sci_calibrated_class])
             .with_meta_targets([SCIENCE])
             .build())
# QC1
