"""
workflow definitions specific to the pupil imaging.
Imports basic image processing from the basic imaging workflow. 
TODO - need N band version
"""

from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule
from .metis_lm_img_wkf import *


lm_pupil_class = classification_rule("LM_PUPIL_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"TECHNICAL", 
                                 "dpr.type":"PUPIL",
                                 "dpr.tech":"PUP,LM",
                                 })


lm_raw_pupil = (data_source()
            .with_classification_rule(lm_pupil_class)        
            .with_match_keywords(["instrume"])
            .build())

pupil_imaging = (task('metis_pupil_imaging')
                    .with_recipe('metis_pupil_imaging')
                    .with_main_input(lm_raw_pupil)
                    .with_associated_input(lingain_2rg_lm_img_task)
                    .with_associated_input(dark_2rg_lm_img_task)
                    .with_associated_input(flat_lm_img_task)
                    .with_meta_targets([SCIENCE])
                    .build())
