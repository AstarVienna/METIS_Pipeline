from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule


dark_class = classification_rule("DARK",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})

#classification_rule("dark",{"instrume":"METIS", "dpr.catg": "CALIB", "dpr.type":"DARK",})
# --- Data sources ---
master_dark = (data_source()
            .with_classification_rule(dark_class)
            .build())


# --- Processing tasks ---
dark_task = (task("metis_det_dark")
            .with_main_input(master_dark)
            .with_meta_targets([SCIENCE])
            .with_recipe("metis_det_dark")
            .build())

