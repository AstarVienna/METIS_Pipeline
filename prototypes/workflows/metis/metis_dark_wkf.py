from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule


dark_class = classification_rule("DARK",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})
detlin_class = classification_rule("DETLIN",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DETLIN",})
lampflat_class = classification_rule("LAMP_FLAT",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"FLAT,LAMP",})
twlightflat_class = classification_rule("TWLIGHT_FLAT",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"FLAT,TWILIGHT",})

obj_class = classification_rule("OBJECT",{"instrume":"METIS", "dpr.catg": "SCIENCE", "dpr.type":"OBJECT",})

#classification_rule("dark",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})
# --- Data sources ---
master_dark = (data_source()
            .with_classification_rule(dark_class)
            .build())

twlight_flat = (data_source()
            .with_classification_rule(lampflat_class)
            .build())


# --- Processing tasks ---
dark_task = (task("metis_det_dark")
            .with_main_input(master_dark)
            .with_meta_targets([SCIENCE])
            .with_recipe("metis_det_dark")
            .build())

