# METIS LSS N BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: 0.0.1
#

from edps import classification_rule


# ----------------------------------------------------------------------------
# ------------------------- Classification rules -----------------------------
# ----------------------------------------------------------------------------

# # RAW data classes +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# RAW data classes
# detlin_class = classification_rule("DETLIN_DET_RAW",
#                                     {"instrume":"METIS",
#                                      "dpr.catg": "CALIB",
#                                      "dpr.type":"DETLIN",
#                                      "dpr.tech":"IMAGE,LM",
#                                      })

lingain_class = classification_rule("DETLIN_GEO_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"DETLIN",
                                     "dpr.tech":"IMAGE,N",
                                     })

geo_wcu_off_class = classification_rule("GEO_WCU_OFF_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"DARK,WCUOFF",
                                     "dpr.tech":"IMAGE,N",
                                     })

dark_class = classification_rule("DARK_GEO_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"FLAT,LAMP",
                                     "dpr.tech":"LSS,N",
                                     })

n_slitloss_class = classification_rule("N_SLITLOSSES_RAW",
                                    {"instrume":"METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type":"SLILOSS",
                                     "dpr.tech":"LSS,N",
                                     })

n_rsrf_class = classification_rule("N_LSS_RSRF_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"CALIB",
                                 "dpr.type":"FLAT,LAMP",
                                 "dpr.tech":"IMAGE,LM",
                                 })

n_rsrf_pinh_class = classification_rule("N_LSS_RSRF_PINH_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.type":"FLAT,LAMP,PINH",
                                 "dpr.tech":"LSS,N",
                                 })

n_raw_std_class = classification_rule("N_LSS_FLUX_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"CALIB",
                                 "dpr.type":"STD",
                                 "dpr.tech":"LSS,N",
                                 })

n_raw_sci_class = classification_rule("N_LSS_SCI_RAW",
                                {"instrume":"METIS",
                                 "dpr.catg":"SCIENCE",
                                 "dpr.type":"OBJECT",
                                 "dpr.tech":"LSS,N",
                                 })
# Final products
n_lss_sci_flux_1d_class = classification_rule("N_LSS_SCI_FLUX_1D",
                                {"instrume":"METIS",
                                 "pro.catg":"N_LSS_SCI_FLUX_1D",
                                 })


# STATIC + OTHER calib classes
static_atm_line_cat_class = classification_rule("ATM_LINE_CAT",
                                {"pro.catg":"ATM_LINE_CAT",
                                 })

static_ref_std_cat_class = classification_rule("REF_STD_CAT",
                                {"pro.catg":"REF_STD_CAT",
                                 })

static_n_lss_dist_sol_class = classification_rule("N_LSS_DIST_SOL",
                                {"pro.catg":"N_LSS_DIST_SOL",
                                 })

static_n_lss_wave_guess_class = classification_rule("N_LSS_WAVE_GUESS",
                                {"pro.catg":"N_LSS_WAVE_GUESS",
                                 })

static_ao_psf_model_class = classification_rule("AO_PSF_MODEL",
                                {"pro.catg":"AO_PSF_MODEL",
                                 })

static_n_adc_slitloss_class = classification_rule("N_ADC_SLITLOSS",
                                {"pro.catg":"N_ADC_SLITLOSS",
                                 })

static_gain_map_geo_class = classification_rule("GAIN_MAP_GEO",
                                {"pro.catg":"GAIN_MAP_GEO",
                                 })

static_linearity_geo_class = classification_rule("LINEARITY_GEO",
                                {"pro.catg":"LINEARITY_GEO",
                                 })

static_badpix_map_geo_class = classification_rule("BADPIX_MAP_GEO",
                                {"pro.catg":"BADPIX_MAP_GEO",
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

