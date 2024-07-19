"""EDPS workflow for METIS, mode IMG_N"""
from edps import task, data_source, classification_rule

# --- Classification rules ---
bpm_class = classification_rule("BADPIX_MAP_GEO",
                                {"instrume": "METIS",
                                 "pro.catg": "BADPIX_MAP_GEO"
                                 })

darkraw_class = classification_rule("DARK_GEO_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type": "DARK",
                                     "dpr.tech": "IMAGE,N"
                                     })

sciraw_class = classification_rule("N_IMAGE_SCI_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "SCIENCE",
                                    "dpr.type": "OBJECT",
                                    "dpr.tech": "IMAGE,N"
                                    })

stdraw_class = classification_rule("N_IMAGE_STD_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "CALIB",
                                    "dpr.type": "STD",
                                    "dpr.tech": "IMAGE,N"
                                    })

fluxstdcat_class = classification_rule("FLUXSTD_CATALOG",
                                       {"pro.catg": "FLUXSTD_CATALOG"}
                                       )


# --- Data sources ---
dark_raw = (data_source()
            .with_classification_rule(darkraw_class)
            .with_match_keywords(["instrume"])
            .build())

sci_n_img_raw = (data_source()
                 .with_classification_rule(sciraw_class)
                 .with_match_keywords(["instrume"])
                 .build())

std_n_img_raw = (data_source()
                 .with_classification_rule(stdraw_class)
                 .with_match_keywords(["instrume"])
                 .build())

fluxstd_cat = (data_source()
               .with_classification_rule(fluxstdcat_class)
               .build())

# --- Processing tasks ---
dark_task = (task("metis_det_dark")
             .with_recipe("metis_det_dark")
             .with_main_input(dark_raw)
             .build())

sci_chopnod_task = (task("chopnod_sci")
                    .with_recipe("metis_n_img_chopnod")
                    .with_main_input(sci_n_img_raw)
                    .build())

std_chopnod_task = (task("chopnod_std")
                    .with_recipe("metis_n_img_chopnod")
                    .with_main_input(std_n_img_raw)
                    .build())

std_process_task = (task("std_process")
                    .with_recipe("metis_n_img_std_process")
                    .with_main_input(std_chopnod_task)
                    .with_associated_input(fluxstd_cat)
                    .build())

calibrate_task = (task("calibrate")
                  .with_recipe("metis_n_img_calibrate")
                  .with_main_input(sci_chopnod_task)
                  .with_associated_input(std_process_task)
                  .build())

restore_task = (task("restore")
                .with_recipe("metis_n_img_restore")
                .with_main_input(calibrate_task)
                .build())
