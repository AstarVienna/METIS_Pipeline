# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import data_source, match_rules
from edps.generator.time_range import *
from .metis_lm_lss_classification import *
from. import metis_lm_lss_keywords as kwd

# ----------------------------------------------------------------------------
# ----------------- Defining required number of input files ------------------
# ----------------------------------------------------------------------------

# NOTE: Currently these values are mostly dummy values!
MIN_NUM_DARKS = 5  # minimum # of darks
MIN_NUM_RSRF = 5
MIN_NUM_PINH = 5
MIN_NUM_SLITLOSPOS = 5 # assuming one file per point source position position (cf. Section 6.8.3 DRLD)

# ----------------------------------------------------------------------------
# ------------------------------- Data sources -------------------------------
# ----------------------------------------------------------------------------

# RAW DATA sources

# TODO:
# Add to data sources (cf. Sect 8 in EDPS tutorial + kmos_datasources.py as example)
# - grouping keywords (".with_grouping_keyords()")
# - matching keywords (".with_matching_keywords()")
# - matching functions (".with_match_function()")
# - setup keywords (".with_setup_keywords()")
# - think about names for the data sources (cf. 5.3. of EDPS Tut)!
# - CHECK what else to add!
# Order of data sources should follow the cascade (cf. Sect 5 in EDPS Tut)

raw_lingain = (data_source()
            .with_classification_rule(lingain_class)
            .with_match_keywords([kwd.instrume])
            .build())

raw_slitloss = (data_source()
            .with_classification_rule(lm_slitloss_class)
            .with_min_group_size(MIN_NUM_SLITLOSPOS)
            .with_match_keywords([kwd.instrume])
            .build())

raw_dark = (data_source()
            .with_classification_rule(dark_class)
            .with_min_group_size(MIN_NUM_DARKS)
            .with_match_keywords([kwd.instrume])
            .build())

lm_raw_rsrf = (data_source()
            .with_classification_rule(lm_rsrf_class)
            .with_min_group_size(MIN_NUM_RSRF)
            .with_match_keywords([kwd.instrume])
            .build())

lm_raw_rsrf_pinh = (data_source()
            .with_classification_rule(lm_rsrf_pinh_class)
            .with_min_group_size(MIN_NUM_PINH)
            .with_match_keywords([kwd.instrume])
            .build())

lm_raw_std = (data_source()
            .with_classification_rule(lm_raw_std_class)
            .with_match_keywords([kwd.instrume])
            .build())

lm_raw_wave = (data_source()
            .with_classification_rule(lm_wave_class)
            .with_match_keywords([kwd.instrume])
            .build())

lm_raw_science = (data_source()
            .with_classification_rule(lm_raw_sci_class)
            .with_match_keywords([kwd.instrume])
            .build())

# Final data products
lm_lss_sci_flux_1d = (data_source()
            .with_classification_rule(lm_lss_sci_flux_1d_class)
            .with_match_keywords([kwd.instrume])
            .build())


# STATIC + OTHER calibration sources
static_gain_map_h2rg = (data_source()
            .with_classification_rule(static_gain_map_h2rg_class)
            .build())

static_linearity_h2rg = (data_source()
            .with_classification_rule(static_linearity_h2rg_class)
            .build())

static_badpix_map_h2rg = (data_source()
            .with_classification_rule(static_badpix_map_h2rg_class)
            .build())

static_persistence_map = (data_source()
            .with_classification_rule(static_persistence_map_class)
            .build())

static_atm_profile = (data_source()
            .with_classification_rule(static_atm_profile_class)
            .build())

static_lsf_kernel = (data_source()
            .with_classification_rule(static_lsf_kernel_class)
            .build())

static_atm_line_cat = (data_source()
            .with_classification_rule(static_atm_line_cat_class)
            .build())

static_laser_tab = (data_source()
            .with_classification_rule(static_laser_tab_class)
            .build())

static_ref_std_cat = (data_source()
            .with_classification_rule(static_ref_std_cat_class)
            .build())

static_lm_lss_dist_sol = (data_source()
            .with_classification_rule(static_lm_lss_dist_sol_class)
            .build())

static_lm_lss_wave_guess = (data_source()
            .with_classification_rule(static_lm_lss_wave_guess_class)
            .build())

static_ao_psf_model = (data_source()
            .with_classification_rule(static_ao_psf_model_class)
            .build())

static_lm_adc_slitloss = (data_source()
            .with_classification_rule(static_lm_adc_slitloss_class)
            .build())

mf_best_fit_tab = (data_source()
            .with_classification_rule(mf_best_fit_tab_class)
            .build())

h2rg_wcu_off = (data_source()
            .with_classification_rule(h2rg_wcu_off_class)
            .build())

