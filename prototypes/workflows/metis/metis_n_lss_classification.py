# METIS LSS N BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import classification_rule
from . import metis_n_lss_keywords as kwd
from . import metis_n_lss_rules as rules


# ----------------------------------------------------------------------------
# ------------------------- Classification rules -----------------------------
# ----------------------------------------------------------------------------


# Define sets of keywords for easier classification / briefer description
metis = {kwd.instrume: "METIS"}
n_lss_calibs = {**metis, kwd.dpr_catg: "CALIB", kwd.dpr_tech: "LSS,N", kwd.det_id: "GEO"}
n_img_calibs = {**metis, kwd.dpr_catg: "CALIB", kwd.dpr_tech: "IMAGE,N", kwd.det_id: "GEO"}

# ----------------------------------------------------------------------------
# RAW data classes +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# ----------------------------------------------------------------------------

lingain_class = classification_rule("DETLIN_GEO_RAW", {**n_img_calibs, kwd.dpr_type:"DETLIN"})

geo_wcu_off_class = classification_rule("GEO_WCU_OFF_RAW", {**n_img_calibs, kwd.dpr_type:"DARK,WCUOFF"})

# TODO: CHECK keyword here!
dark_class = classification_rule("DARK_GEO_RAW", {**n_img_calibs, kwd.dpr_type:"DARK,WCUON"})

n_slitloss_class = classification_rule("N_SLITLOSSES_RAW", {**n_lss_calibs, kwd.dpr_type:"SLILOSS"})

n_rsrf_class = classification_rule("N_LSS_RSRF_RAW", {**n_img_calibs, kwd.dpr_type:"FLAT,LAMP"})

n_rsrf_pinh_class = classification_rule("N_LSS_RSRF_PINH_RAW", {**n_lss_calibs, kwd.dpr_type:"FLAT,LAMP,PINH"})

n_raw_std_class = classification_rule("N_LSS_FLUX_RAW", {**n_lss_calibs, kwd.dpr_type:"STD"})

n_raw_sci_class = classification_rule("N_LSS_SCI_RAW",
                                {**metis,
                                 kwd.dpr_catg:"SCIENCE",
                                 kwd.dpr_type:"OBJECT",
                                 kwd.dpr_tech:"LSS,N",
                                 })

# ----------------------------------------------------------------------------
# MASTER and intermediate calibrations classes -------------------------------
# ----------------------------------------------------------------------------
# TODO: Check the correct usage of that in EDPS!
MASTERDARK_class = classification_rule("MASTERDARK", {**metis, kwd.pro_catg:"MASTERDARK", kwd.det_id: "GEO"})


# ----------------------------------------------------------------------------
# Final products data classes ------------------------------------------------
# ----------------------------------------------------------------------------
n_lss_sci_flux_1d_class = classification_rule("N_LSS_SCI_FLUX_1D", {**metis, kwd.pro_catg:"N_LSS_SCI_FLUX_1D"})

# ----------------------------------------------------------------------------
# STATIC + OTHER/EXTERNAL calib classes
# ----------------------------------------------------------------------------
# Atmospheric line catalogue
static_atm_line_cat_class = classification_rule("ATM_LINE_CAT", {kwd.pro_catg:"ATM_LINE_CAT"})
# Catalogue of standard stars
static_ref_std_cat_class = classification_rule("REF_STD_CAT", {kwd.pro_catg:"REF_STD_CAT"})
# Distortion solution
static_n_lss_dist_sol_class = classification_rule("N_LSS_DIST_SOL", {kwd.pro_catg:"N_LSS_DIST_SOL"})
# First wavelength calibration guess
static_n_lss_wave_guess_class = classification_rule("N_LSS_WAVE_GUESS", {kwd.pro_catg:"N_LSS_WAVE_GUESS"})
# Static PSF model
static_ao_psf_model_class = classification_rule("AO_PSF_MODEL", {kwd.pro_catg:"AO_PSF_MODEL"})
# ADC Slitloss file
static_n_adc_slitloss_class = classification_rule("N_ADC_SLITLOSS", {kwd.pro_catg:"N_ADC_SLITLOSS"})
# Static GAIN map
static_gain_map_geo_class = classification_rule("GAIN_MAP_GEO", {kwd.pro_catg:"GAIN_MAP_GEO"})
# Linearity file
static_linearity_geo_class = classification_rule("LINEARITY_GEO", {kwd.pro_catg:"LINEARITY_GEO"})
# Bad pixel map
static_badpix_map_geo_class = classification_rule("BADPIX_MAP_GEO", {kwd.pro_catg:"BADPIX_MAP_GEO"})
# Persistence map class (actually NOT a static file, but from external source)
static_persistence_map_class = classification_rule("PERSISTENCE_MAP", {kwd.pro_catg:"PERSISTENCE_MAP"})
# Atmospheric profiles (for molecfit)
static_atm_profile_class = classification_rule("ATM_PROFILE", {kwd.pro_catg:"ATM_PROFILE"})
# Line spread function class
static_lsf_kernel_class = classification_rule("LSF_KERNEL", {kwd.pro_catg:"LSF_KERNEL"})
# Table for best-fit molecfit parameters
mf_best_fit_tab_class = classification_rule("MF_BEST_FIT_TAB", {kwd.pro_catg:"MF_BEST_FIT_TAB"})



