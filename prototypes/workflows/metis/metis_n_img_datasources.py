"""Data sources for metis_n_img_wkf.py"""
from edps import data_source
from .metis_n_img_classification import *

detlin_on = (data_source()
             .with_classification_rule(detlin_on_class)
             .with_match_keywords(["instrume"])
             .build())

detlin_off = (data_source()
              .with_classification_rule(detlin_off_class)
              .with_match_keywords(["instrume"])
              .build())

dark_raw = (data_source()
            .with_classification_rule(darkraw_class)
            .with_match_keywords(["instrume"])
            .build())

flat_lamp_raw = (data_source()
                 .with_classification_rule(flat_lamp_class)
                 .with_match_keywords(["instrume"])
                 .build())

flat_twilight_raw = (data_source()
                     .with_classification_rule(flat_twilight_class)
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
