# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import classification_rule
from . import metis_lm_lss_keywords as metis_kwd
from . import metis_lm_lss_rules as rules

# ----------------------------------------------------------------------------
# ------------------------- Classification rules -----------------------------
# ----------------------------------------------------------------------------

# Define sets of keywords for easier classification / briefer description
# These sets replace individual keyword lists in classifications (cf. kmos_classification.py)
lm_lss_calibs = {metis_kwd.instrume: "METIS", metis_kwd.dpr_catg: "CALIB", metis_kwd.dpr_tech: "LSS,LM", metis_kwd.det_id: "2RG"}
# lm_img_calibs = {metis_kwd.instrume: "METIS", metis_kwd.dpr_catg: "CALIB", metis_kwd.dpr_tech: "IMAGE,LM", metis_kwd.det_id: "2RG"}

# TODO: Add more of these sets and replace them in the classification!

# ----------------------------------------------------------------------------
# RAW data classes -----------------------------------------------------------
# ----------------------------------------------------------------------------
# Linearity file class
detlin_2rg_raw_class = classification_rule("DETLIN_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "IMAGE,LM",
     # metis_kwd.det_id: "2RG",
     metis_kwd.dpr_type:"DETLIN"})

# # DARK class
rawdark_class = classification_rule("DARK_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "IMAGE,LM",
     metis_kwd.dpr_type:"DARK"})

# Class for WCU OFF files
lm_wcu_off_raw_class = classification_rule("LM_WCU_OFF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "IMAGE,LM",
     # metis_kwd.det_id: "2RG",
     metis_kwd.dpr_type:"DARK,WCUOFF"})

# Slitloss files (TODO: Check the difference to STATIC ones - doubly defined??  Also check whether img or lss mode)
lm_slitloss_class = classification_rule("LM_ADC_SLITLOSS_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     # metis_kwd.det_id: "2RG",
     metis_kwd.dpr_type:"SLITLOSS"})

# RSRF / FLATFIELDS
lm_rsrf_raw_class = classification_rule("LM_LSS_RSRF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     # metis_kwd.det_id: "2RG",
     metis_kwd.dpr_type:"FLAT,LAMP"})

# RSRF Pinhole frames
lm_rsrf_pinh_class = classification_rule("LM_LSS_RSRF_PINH_RAW", {metis_kwd.instrume: "METIS", metis_kwd.dpr_catg: "CALIB", metis_kwd.dpr_tech: "LSS,LM",  metis_kwd.dpr_type:"FLAT,LAMP,PINH"})
# Wavelength calib files (WCU laser sources)
lm_wave_class = classification_rule("LM_LSS_WAVE_RAW", {metis_kwd.instrume: "METIS", metis_kwd.dpr_catg: "CALIB", metis_kwd.dpr_tech: "LSS,LM",  metis_kwd.dpr_type:"WAVE"})
# Standrad stars raw
lm_raw_std_class = classification_rule("LM_LSS_FLUX_RAW", {metis_kwd.instrume: "METIS", metis_kwd.dpr_catg: "CALIB", metis_kwd.dpr_tech: "LSS,LM",  metis_kwd.dpr_type:"STD"})
# Sci obs raw
lm_raw_sci_class = classification_rule("LM_LSS_SCI_RAW", {metis_kwd.instrume: "METIS",
                                 metis_kwd.dpr_catg:"SCIENCE",
                                 metis_kwd.dpr_type:"OBJECT",
                                 metis_kwd.dpr_tech:"LSS,LM"})

# ----------------------------------------------------------------------------
# MASTER and intermediate calibrations classes -------------------------------
# ----------------------------------------------------------------------------

# TODO: Check the correct usage of that in EDPS!
MASTERDARK_class = classification_rule("MASTERDARK", {metis_kwd.instrume: "METIS", metis_kwd.pro_catg:"MASTERDARK", metis_kwd.det_id: "2RG"})

# ----------------------------------------------------------------------------
# FINAL producta data classes ------------------------------------------------
# ----------------------------------------------------------------------------
lm_lss_sci_flux_1d_class = classification_rule("LM_LSS_SCI_FLUX_1D", {metis_kwd.instrume: "METIS",metis_kwd.pro_catg:"LM_LSS_SCI_FLUX_1D"})

# ----------------------------------------------------------------------------
# STATIC + OTHER/EXTERNAL calib classes --------------------------------------
# ----------------------------------------------------------------------------
# Atmospheric line catalogue
static_atm_line_cat_class = classification_rule("ATM_LINE_CAT", {metis_kwd.pro_catg:"ATM_LINE_CAT"})
# Table with WCU laser wavelengthes
static_laser_tab_class = classification_rule("LASER_TAB", {metis_kwd.pro_catg:"LASER_TAB"})
# Catalogue of standard stars
static_ref_std_cat_class = classification_rule("REF_STD_CAT", {metis_kwd.pro_catg:"REF_STD_CAT"})
# Distortion solution
static_lm_lss_dist_sol_class = classification_rule("LM_LSS_DIST_SOL", {metis_kwd.pro_catg:"LM_LSS_DIST_SOL",})
# First wavelength calibration guess
static_lm_lss_wave_guess_class = classification_rule("LM_LSS_WAVE_GUESS", {metis_kwd.pro_catg:"LM_LSS_WAVE_GUESS"})
# Static PSF model
static_ao_psf_model_class = classification_rule("AO_PSF_MODEL", {metis_kwd.pro_catg:"AO_PSF_MODEL"})
# ADC Slitloss file
static_lm_adc_slitloss_class = classification_rule("LM_ADC_SLITLOSS", {metis_kwd.pro_catg:"LM_ADC_SLITLOSS"})
# Static GAIN map
static_gain_map_h2rg_class = classification_rule("GAIN_MAP_2RG", {metis_kwd.pro_catg:"GAIN_MAP_2RG"})
# Linearity file
static_linearity_h2rg_class = classification_rule("LINEARITY_2RG", {metis_kwd.pro_catg:"LINEARITY_2RG"})
# Bad pixel map
static_badpix_map_h2rg_class = classification_rule("BADPIX_MAP_2RG", {metis_kwd.pro_catg:"BADPIX_MAP_2RG"})
# Persistence map class (actually NOT static, but from external source)
static_persistence_map_class = classification_rule("PERSISTENCE_MAP", {metis_kwd.pro_catg:"PERSISTENCE_MAP"})
# Atmospheric profiles (for molecfit)
static_atm_profile_class = classification_rule("ATM_PROFILE", {metis_kwd.pro_catg:"ATM_PROFILE"})
# Line spread function class
static_lsf_kernel_class = classification_rule("LSF_KERNEL", {metis_kwd.pro_catg:"LSF_KERNEL"})
# Table for best-fit molecfit parameters
mf_best_fit_tab_class = classification_rule("MF_BEST_FIT_TAB", {metis_kwd.pro_catg:"MF_BEST_FIT_TAB"})
