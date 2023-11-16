from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, ReportInput
from edps import data_source
from edps.generator.time_range import *
from edps import classification_rule
from edps import match

### metis_keywords.py ###
# HEADER KEYWORDS USED FOR CLASSIFICATION, GROUPING, AND ASSOCIATION.
instrume = "instrume"
pro_catg = "pro.catg"
dpr_type = "dpr.type"
dpr_catg = "dpr.catg"
dpr_tech = "dpr.tech"
tpl_nexp = "tpl.nexp"
obs_id = "obs.id"
tpl_start = "tpl.start"
det_binx = "det.binx"
det_biny = "det.biny"
ins_mode = "ins.mode"
unique = "arcfile"
mjd_obs = "mjd-obs"

### metis_rules.py ###

def is_metis(f):
    return f[instrume] == "METIS"

def is_calib(f):
    return is_metis() and f[dpr_catg] == "CALIB"

def is_science(f):
    return is_metis() and f[dpr_catg] == "SCIENCE"

# ASSOCIATION RULES f=file to associate (e.g. calibration) ref=trigger (e.g. science)
# All the associations but this one are specified in the workflow, using .with_match_keywords()

# This function forces unconditional association
def is_associated(ref, f):
    return True


def assoc_setup_and_telescope(ref, f):
    # return match(ref, f, [kwd.det_binx, kwd.det_biny, kwd.ins_mode, kwd.instrume]) and \
    #        (match(ref, f, [kwd.ins5_modsel_id]) or match(ref, f, [kwd.ocs_enabled_fe]))
    return match(ref, f, [kwd.instrume])


### metis_classification.py ###
#from . import metis_keywords as kwd
#from . import aspresso_rules as rules

# Dictionaries containing the values of header keywords that define calibrations and science data
metis_keywords = {instrume: "ESPRESSO"}
calib_keywords = {**metis_keywords, dpr_catg: "CALIB"}
science_keywords = {**metis_keywords, dpr_catg: "SCIENCE"}

# Raw types
n_image_sci_raw_class = classification_rule("N_IMAGE_SCI_RAW", {
    dpr_catg: "SCIENCE",
    dpr_tech: "IMAGE,N",
    dpr_type: "OBJECT",
})

# Master calibrations
master_dark_geo_class = classification_rule("MASTER_DARK_GEO", {pro_catg: "MASTER_DARK_GEO"})
master_flat_geo_class = classification_rule("MASTER_FLAT_GEO", {pro_catg: "MASTER_FLAT_GEO"})



### metis_datasoources.py ###
#from .metis_classification import *

n_image_sci_raw = (data_source()
                   .with_classification_rule(n_image_sci_raw_class)
                   #.with_grouping_keywords([tpl_start])
                   #.with_setup_keywords()
                   .build())

# and external data
# list master dark and flat as external data for now
master_dark_geo = (data_source()
                   .with_classification_rule(master_dark_geo_class)
                   .build())
master_flat_geo = (data_source()
                   .with_classification_rule(master_flat_geo_class)
                   .build())



### metis_wkf.py ###
#from .metis_datasources import *

my_basic_science = (task('my_basic_science')
                    .with_recipe('basic_science')
                    .with_main_input(n_image_sci_raw)
                    .with_associated_input(master_dark_geo)
                    #.with_associated_input(master_flat_geo)
                    .with_meta_targets([SCIENCE])
                    .build())
