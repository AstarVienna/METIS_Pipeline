# Hallo Wolfi
"""METIS LSS N-Band workflow"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule

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

# --- Data sources ---
# STATIC + OTHER calibration sources
static_gain_map_geo = (data_source()
            .with_classification_rule(static_gain_map_geo_class)
            .build())

static_linearity_geo = (data_source()
            .with_classification_rule(static_linearity_geo_class)
            .build())

geo_wcu_off = (data_source()
            .with_classification_rule(geo_wcu_off_class)
            .build())

static_badpix_map_geo = (data_source()
            .with_classification_rule(static_badpix_map_geo_class)
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

static_ref_std_cat = (data_source()
            .with_classification_rule(static_ref_std_cat_class)
            .build())

static_n_lss_dist_sol = (data_source()
            .with_classification_rule(static_n_lss_dist_sol_class)
            .build())

static_n_lss_wave_guess = (data_source()
            .with_classification_rule(static_n_lss_wave_guess_class)
            .build())

static_ao_psf_model = (data_source()
            .with_classification_rule(static_ao_psf_model_class)
            .build())

static_n_adc_slitloss = (data_source()
            .with_classification_rule(static_n_adc_slitloss_class)
            .build())

mf_best_fit_tab = (data_source()
            .with_classification_rule(mf_best_fit_tab_class)
            .build())


# RAW DATA sources

raw_lingain = (data_source()
            .with_classification_rule(lingain_class)
            .with_match_keywords(["instrume"])
            .build())

raw_slitloss = (data_source()
            .with_classification_rule(n_slitloss_class)
            .with_match_keywords(["instrume"])
            .build())

raw_dark = (data_source()
            .with_classification_rule(dark_class)
            .with_match_keywords(["instrume"])
            .build())

n_raw_rsrf = (data_source()
            .with_classification_rule(n_rsrf_class)
            .with_match_keywords(["instrume"])
            .build())

n_raw_rsrf_pinh = (data_source()
            .with_classification_rule(n_rsrf_pinh_class)
            .with_match_keywords(["instrume"])
            .build())

n_raw_std = (data_source()
            .with_classification_rule(n_raw_std_class)
            .with_match_keywords(["instrume"])
            .build())

n_raw_science = (data_source()
            .with_classification_rule(n_raw_sci_class)
            .with_match_keywords(["instrume"])
            .build())

n_lss_sci_flux_1d = (data_source()
            .with_classification_rule(n_lss_sci_flux_1d_class)
            .with_match_keywords(["instrume"])
            .build())


# --- Processing tasks ---
# persistence_task = (task('metis_det_persistence')
#             .with_main_input(raw_slitloss)
#             .with_recipe("metis_det_persistence")
#             .build())

lingain_task = (task('metis_det_lingain')
            .with_main_input(raw_lingain)
            .with_associated_input(geo_wcu_off)
            .with_recipe("metis_det_lingain")
            .with_meta_targets([QC1_CALIB])
            .build())

slitloss_task = (task('metis_n_adc_slitloss')
            .with_main_input(raw_slitloss)
            .with_recipe("metis_n_adc_slitloss")
            .with_meta_targets([QC1_CALIB])
            .build())

dark_task = (task('metis_det_dark')
            .with_main_input(raw_dark)
            .with_recipe("metis_det_dark")
            .with_meta_targets([QC1_CALIB])
            .build())

rsrf_task = (task('metis_N_lss_rsrf')
            .with_main_input(n_raw_rsrf)
            .with_associated_input(dark_task)
            .with_recipe("metis_N_lss_rsrf")
            .with_meta_targets([QC1_CALIB])
            .build())

trace_task = (task("metis_N_lss_trace")
            .with_main_input(n_raw_rsrf_pinh)
            .with_associated_input(dark_task)
            .with_recipe("metis_N_lss_trace")
            .with_meta_targets([QC1_CALIB])
            .build())

std_reduction = (task('metis_N_lss_std')
            .with_recipe('metis_N_lss_std')
            .with_main_input(n_raw_std)
            # .with_associated_input(lingain_task)
            .with_associated_input(static_badpix_map_geo)
            .with_associated_input(static_gain_map_geo)
            .with_associated_input(static_linearity_geo)
            .with_associated_input(static_persistence_map)
            .with_associated_input(dark_task)
            .with_associated_input(rsrf_task)
            .with_associated_input(trace_task)
            .with_associated_input(slitloss_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_ref_std_cat)
            .with_associated_input(static_n_lss_dist_sol)
            .with_associated_input(static_n_lss_wave_guess)
            # .with_associated_input(static_n_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_meta_targets([SCIENCE])
            .build())

sci_reduction = (task('metis_N_lss_sci')
            .with_recipe('metis_N_lss_sci')
            .with_main_input(n_raw_science)
            # .with_associated_input(lingain_task)
            .with_associated_input(static_badpix_map_geo)
            .with_associated_input(static_gain_map_geo)
            .with_associated_input(static_linearity_geo)
            .with_associated_input(static_persistence_map)
            .with_associated_input(dark_task)
            .with_associated_input(rsrf_task)
            .with_associated_input(trace_task)
            # .with_associated_input(slitloss_task)
            .with_associated_input(static_n_lss_dist_sol)
            .with_associated_input(static_n_lss_wave_guess)
            .with_associated_input(std_reduction)
            .with_associated_input(static_atm_line_cat)
            # .with_associated_input(static_n_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_meta_targets([SCIENCE])
            .build())

mf_model_task = (task("metis_N_lss_mf_model")
            .with_main_input(sci_reduction)
            .with_associated_input(static_ao_psf_model)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_N_lss_mf_model")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_calctrans_task = (task("metis_N_lss_calctrans")
            .with_main_input(mf_model_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_N_lss_calctrans")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_correct_task = (task("metis_N_lss_mf_correct")
            .with_main_input(mf_calctrans_task)
            .with_associated_input(mf_calctrans_task)
            .with_recipe("metis_N_lss_mf_correct")
            .with_meta_targets([QC1_CALIB])
            .build())




