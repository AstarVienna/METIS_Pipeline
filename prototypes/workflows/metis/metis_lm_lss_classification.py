# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import classification_rule
from . import metis_lm_lss_keywords as kwd

# ----------------------------------------------------------------------------
# ------------------------- Classification rules -----------------------------
# ----------------------------------------------------------------------------

# RAW data classes +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# detlim_class = classification_rule("DETLIN_DET_RAW",
#                                     {"instrume":"METIS",
#                                      "dpr.catg": "CALIB",
#                                      "dpr.type":"DETLIN",
#                                      "dpr.tech":"IMAGE,LM",
#                                      })


lingain_class = classification_rule("DETLIN_2RG_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"DETLIN",
                                     "dpr.tech":"IMAGE,LM",
                                     })

h2rg_wcu_off_class = classification_rule("DETLIN_2RG_OFF_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"DARK,WCUOFF",
                                     "dpr.tech":"IMAGE,LM",
                                     })

dark_class = classification_rule("DARK_2RG_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"DARK",
                                     "dpr.tech":"LSS,LM",
                                     })

lm_slitloss_class = classification_rule("LM_SLITLOSSES_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"SLILOSS",
                                     "dpr.tech":"LSS,LM",
                                     })

lm_rsrf_class = classification_rule("LM_LSS_RSRF_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"CALIB",
                                 "dpr.type":"FLAT,LAMP",
                                 "dpr.tech":"IMAGE,LM",
                                 })

lm_rsrf_pinh_class = classification_rule("LM_LSS_RSRF_PINH_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.type":"FLAT,LAMP,PINH",
                                 "dpr.tech":"LSS,LM",
                                 })

lm_wave_class = classification_rule("LM_LSS_WAVE_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.type":"WAVE",
                                 "dpr.tech":"LSS,LM",
                                 })

lm_raw_std_class = classification_rule("LM_LSS_FLUX_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"CALIB",
                                 "dpr.type":"STD",
                                 "dpr.tech":"LSS,LM",
                                 })

lm_raw_sci_class = classification_rule("LM_LSS_SCI_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"SCIENCE",
                                 "dpr.type":"OBJECT",
                                 "dpr.tech":"LSS,LM",
                                 })
# Final products
lm_lss_sci_flux_1d_class = classification_rule("LM_LSS_SCI_FLUX_1D",
                                {"instrume":"METIS",
                                 "pro.catg":"LM_LSS_SCI_FLUX_1D",
                                 })


# STATIC + OTHER calib classes +++++++++++++++++++++++++++++++++++++++++++++++

static_atm_line_cat_class = classification_rule("ATM_LINE_CAT",
                                {"pro.catg":"ATM_LINE_CAT",
                                 })

static_laser_tab_class = classification_rule("LASER_TAB",
                                {"pro.catg":"LASER_TAB",
                                 })

static_ref_std_cat_class = classification_rule("REF_STD_CAT",
                                {"pro.catg":"REF_STD_CAT",
                                 })

static_lm_lss_dist_sol_class = classification_rule("LM_LSS_DIST_SOL",
                                {"pro.catg":"LM_LSS_DIST_SOL",
                                 })

static_lm_lss_wave_guess_class = classification_rule("LM_LSS_WAVE_GUESS",
                                {"pro.catg":"LM_LSS_WAVE_GUESS",
                                 })

static_ao_psf_model_class = classification_rule("AO_PSF_MODEL",
                                {"pro.catg":"AO_PSF_MODEL",
                                 })

static_lm_adc_slitloss_class = classification_rule("LM_ADC_SLITLOSS",
                                {"pro.catg":"LM_ADC_SLITLOSS",
                                 })

static_gain_map_h2rg_class = classification_rule("GAIN_MAP_2RG",
                                {"pro.catg":"GAIN_MAP_2RG",
                                 })

static_linearity_h2rg_class = classification_rule("LINEARITY_2RG",
                                {"pro.catg":"LINEARITY_2RG",
                                 })

static_badpix_map_h2rg_class = classification_rule("BADPIX_MAP_2RG",
                                {"pro.catg":"BADPIX_MAP_2RG",
                                 })

static_persistence_map_class = classification_rule("PERSISTENCE_MAP",
                                {"pro.catg":"PERSISTENCE_MAP",
                                 })

static_atm_profile_class = classification_rule("ATM_PROFILE",
                                {"pro.catg":"ATM_PROFILE",
                                 })

static_lsf_kernel_class = classification_rule("LSF_KERNEL",
                                {"pro.catg":"LSF_KERNEL",
                                 })

mf_best_fit_tab_class = classification_rule("MF_BEST_FIT_TAB",
                                {"pro.catg":"MF_BEST_FIT_TAB",
                                 })
