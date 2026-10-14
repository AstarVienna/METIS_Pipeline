"""
workflow definitions specific to the pupil imaging.
Imports basic image processing from the basic imaging workflow.
TODO - need N band version
"""

from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule
from .metis_classification import (lm_pupil_raw_class, n_pupil_raw_class, linearity_2rg_class, gain_map_2rg_class,
                                   master_dark_2rg_class, master_img_flat_lamp_lm_class)
from .metis_lm_img_wkf import lm_img_lingain_task, lm_img_dark_task, lm_img_flat_task






lm_raw_pupil = (data_source()
            .with_classification_rule(lm_pupil_raw_class)
            .with_match_keywords(["instrume"])
            .build())


pupil_imaging = (task('metis_pupil_imaging')
                    .with_recipe('metis_pupil_imaging')
                    .with_main_input(lm_raw_pupil)
                    .with_associated_input(lm_img_lingain_task, [linearity_2rg_class, gain_map_2rg_class])
                    .with_associated_input(lm_img_dark_task, [master_dark_2rg_class])
                    .with_associated_input(lm_img_flat_task, [master_img_flat_lamp_lm_class])
                    .with_meta_targets([SCIENCE])
                    .build())
