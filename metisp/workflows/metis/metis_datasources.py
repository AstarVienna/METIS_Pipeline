from edps import data_source
from edps.generator.time_range import *

from .metis_classification import *

# Convention for Data sources Association rule levels:
# Each data source can have several match function which correspond to different
# quality levels for the selected data. The level is specified as a number that
# follows this convention:
#   level < 0: more restrictive than the calibration plan
#   level = 0 follows the calibration plan
#   level = 1 quality sufficient for QC1 certification
#   level = 2 probably still acceptable quality
#   level = 3 significant risk of bad quality results

# standard matching keywords:
setup = [metis_kwd.det_binx, metis_kwd.det_biny, metis_kwd.ins_mode]
instrument_setup = [metis_kwd.instrume] + setup


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
