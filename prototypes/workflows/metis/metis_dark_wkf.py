from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule


dark_class = classification_rule("DARK_LM_RAW",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})

detlin_class = classification_rule("DETLIN",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DETLIN",})

lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
                                {"instrume":"METIS", 
                                 "dpr.catg":"CALIB", 
                                 "dpr.type":"FLAT,LAMP",
                                 "dpr.tech":"IMAGE,N",
                                 })

twlightflat_class = classification_rule("TWLIGHT_FLAT",
                                {"instrume":"METIS", 
                                 "dpr.catg": "CALIB", 
                                 "dpr.type":"FLAT,TWILIGHT",})

masterdark_class = classification_rule("MASTER_DARK_2RG",
                                {"instrume":"METIS", 
                                 "pipefile": "MASTER_DARK_2RG.fits", 
                                 })



obj_class = classification_rule("OBJECT",{"instrume":"METIS", "dpr.catg": "SCIENCE", "dpr.type":"OBJECT",})

#classification_rule("dark",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})
# --- Data sources ---
master_dark = (data_source()
            .with_classification_rule(master_class)
            .build())

lm_lampclass_flat = (data_source()
            .with_classification_rule(lm_lampflat_class)
            .build())


# --- Processing tasks ---
dark_task = (task("metis_det_dark")
            .with_main_input(master_dark)
            .with_meta_targets([SCIENCE])
            .with_recipe("metis_det_dark")
            .build())

flat_task = (task("metis_lm_img_flat")
            .with_main_input(lm_lampclass_flat)
            .with_associated_input(dark_task, [masterdark_class])
            .with_recipe("metis_lm_img_flat")
            .build())

