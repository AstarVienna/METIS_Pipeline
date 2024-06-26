from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule

detlin_class = classification_rule("DETLIN_DET_RAW",
                                    {"instrume":"METIS", 
                                     "dpr.catg": "CALIB", 
                                     "dpr.type":"DETLIN",
                                     "dpr.tech":"IMAGE,LM",
                                     })

rawdark_class = classification_rule("DARK_LM_RAW",
                                    {"instrume":"METIS", 
                                     "dpr.catg": "CALIB", 
                                     "dpr.type":"DARK",
                                     "dpr.tech":"IMAGE,LM",
                                     })

lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"CALIB", 
                                 "dpr.type":"FLAT,LAMP",
                                 "dpr.tech":"IMAGE,LM",
                                 })

twlightflat_class = classification_rule("TWLIGHT_FLAT",
                                {"instrume":"METIS", 
                                 "dpr.catg": "CALIB", 
                                 "dpr.type":"FLAT,TWILIGHT",
                                 "dpr.tech":"IMAGE,LM",
                                 })


raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"SCIENCE", 
                                 "dpr.type":"OBJECT",
                                 "dpr.tech":"IMAGE,LM",
                                 })


# --- Data sources ---
detlin_raw = (data_source()
            .with_classification_rule(detlin_class)
            .with_match_keywords(["instrume"])
            .build())

raw_dark = (data_source()
            .with_classification_rule(rawdark_class)
            .with_match_keywords(["instrume"])
            .build())

lm_lamp_flat = (data_source()
            .with_classification_rule(lm_lampflat_class)
            .with_match_keywords(["instrume"])
            .build())

lm_raw_science = (data_source()
            .with_classification_rule(raw_science_class)        
            .with_match_keywords(["instrume"])
            .build())

# --- Processing tasks ---
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

basic_reduction = (task('basic_reduction')
                    .with_recipe('metis_lm_basic_reduction')
                    .with_main_input(lm_raw_science)
                    .with_associated_input(lingain_task)
                    .with_associated_input(dark_task)
                    .with_associated_input(flat_task)
                    .with_meta_targets([SCIENCE])
                    .build())
