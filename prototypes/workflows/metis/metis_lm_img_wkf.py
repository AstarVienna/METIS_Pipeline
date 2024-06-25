from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule

detlin_class = classification_rule("DETLIN",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DETLIN",})

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
                                 "dpr.type":"FLAT,TWILIGHT",})

masterdark_class = classification_rule("MASTER_DARK_2RG",
                                {"instrume":"METIS", 
                                 "pipefile": "MASTER_DARK_2RG.fits", 
                                 })

masterflat_class = classification_rule("MASTER_FLAT_LAMP",
                                {"instrume":"METIS", 
                                 "pipefile": "MASTER_IMG_FLAT_LAMP.fits", 
                                 })

raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"SCIENCE", 
                                 "dpr.type":"OBJECT",
                                 "dpr.tech":"IMAGE,LM",
                                 })


#classification_rule("dark",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})
# --- Data sources ---
raw_dark = (data_source()
            .with_classification_rule(rawdark_class)
            .with_match_keywords(["instrume"])
            .build())

lm_lampclass_flat = (data_source()
            .with_classification_rule(lm_lampflat_class)
            .with_match_keywords(["instrume"])
            .build())

lm_basic_science = (data_source()
            .with_classification_rule(raw_science_class)        
            .with_match_keywords(["instrume"])
            .build())

# --- Processing tasks ---
dark_task = (task('metis_det_dark')
            .with_main_input(raw_dark)
            .with_recipe("metis_det_dark")
            .build())

flat_task = (task("metis_lm_img_flat")
            .with_main_input(lm_lampclass_flat)
            .with_associated_input(dark_task,[masterdark_class])
            .with_recipe("metis_lm_img_flat")
            .build())

basic_science = (task('basic_science')
                    .with_recipe('metis_lm_basic_science')
                    .with_main_input(lm_basic_science)
                    .with_associated_input(dark_task,[masterdark_class])
                    .with_associated_input(flat_task, [masterflat_class])
                    .with_meta_targets([SCIENCE])
                    .build())
