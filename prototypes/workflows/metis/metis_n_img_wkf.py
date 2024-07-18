"""EDPS workflow for METIS, mode IMG_N"""
from edps import task, data_source, classification_rule

# --- Classification rules ---
darkraw_class = classification_rule("DARK_GEO_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type": "DARK",
                                     "dpr.tech": "IMAGE,N"
                                     })




# --- Data sources ---
dark_raw = (data_source()
            .with_classification_rule(darkraw_class)
            .with_match_keywords(["instrume"])
            .build())


# --- Processing tasks ---
dark_task = (task("metis_det_dark")
             .with_main_input(dark_raw)
             .with_recipe("metis_det_dark")
             .build())
