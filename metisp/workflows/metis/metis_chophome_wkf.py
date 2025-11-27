"""
workflow definitions specific to the pupil imaging.
Imports basic image processing from the basic imaging workflow. 
TODO - need N band version
"""

from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule
from .metis_datasources import *
from . import metis_keywords as metis_kwd


lm_chophome_class = classification_rule("LM_CHOPHOME_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"CALIB", 
                                 "dpr.type":"CHOPHOME",
                                 "dpr.tech":"IMAGE,LM",
                                 })


lm_raw_chophome = (data_source()
            .with_classification_rule(lm_chophome_class)        
            .with_match_keywords(["instrume"])
            .build())

metis_chophome_imaging = (task('metis_chophome_imaging')
                    .with_recipe('metis_cal_chophome')
                    .with_main_input(lm_raw_chophome)
                    .with_associated_input(persistence_map)
                    .with_associated_input(lm_wcu_off_raw)
                    .with_meta_targets([SCIENCE])
                    .build())
