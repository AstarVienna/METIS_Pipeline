from edps import classification_rule
from . import metis_keywords as metis_kwd

# Detector linearity calibration classification
detlin_2rg_raw_class = classification_rule("DETLIN_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DETLIN",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# Dark frame calibration classification
rawdark_2rg_class = classification_rule("DARK_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_distortion_raw_class = classification_rule("LM_DISTORTION_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DISTORTION",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_wcu_off_raw_class = classification_rule("LM_WCU_OFF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK,WCUOFF",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })


# Lamp flat calibration classification
lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# Twilight flat calibration classification
lm_twilightflat_class = classification_rule("LM_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# Science observation classification
lm_raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_raw_sky_class = classification_rule("LM_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_raw_std_class = classification_rule("LM_IMAGE_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "STD",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# ------- N BAND CLASSIFICATIONS -----------


# Detector linearity calibration classification
detlin_geo_raw_class = classification_rule("DETLIN_GEO_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DETLIN",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

# Dark frame calibration classification
rawdark_geo_class = classification_rule("DARK_GEO_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_distortion_raw_class = classification_rule("N_DISTORTION_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DISTORTION",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_wcu_off_raw_class = classification_rule("N_WCU_OFF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK,WCUOFF",
     metis_kwd.dpr_tech: "IMAGE,N",
    })


# Lamp flat calibration classification
n_lampflat_class = classification_rule("N_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

# Twilight flat calibration classification
n_twilightflat_class = classification_rule("N_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

# Science observation classification
n_raw_science_class = classification_rule("N_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_raw_sky_class = classification_rule("N_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_raw_std_class = classification_rule("N_IMAGE_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "STD",
     metis_kwd.dpr_tech: "IMAGE,N",
    })



# Flux standard catalog classification
fluxstd_catalog_class = classification_rule("FLUXSTD_CATALOG",
     {metis_kwd.pro_catg: "FLUXSTD_CATALOG",
    })

# Pinhole table classification
pinhole_table_class = classification_rule("PINHOLE_TABLE",
    {metis_kwd.pro_catg: "PINHOLE_TABLE",
    })

# --- IFU Classifications ---

badpix_ifu_class = classification_rule("BADPIX_MAP_IFU",
                                   {"pro.catg": "BADPIX_MAP_det",
                                    })

detlin_ifu_class = classification_rule("DETLIN_IFU_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "CALIB",
                                    "dpr.type": "DETLIN",
                                    "dpr.tech": "IFU",                                    
                                    })

gain_map_ifu_class = classification_rule("GAIN_MAP_IFU",
                                     {"pro.catg": "GAIN_MAP_det",
                                      })

linearity_det_ifu_class = classification_rule("LINEARITY_IFU",
                                    {"pro.catg": "LINEARITY_det",
                                    })

rawdark_ifu_class = classification_rule("DARK_IFU_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.tech": "IFU",
                                     "dpr.type": "DARK",
                                     })

distortion_ifu_class = classification_rule("IFU_DISTORTION_RAW",
                                       {"instrume": "METIS",
                                        "dpr.catg": "CALIB",
                                        "dpr.tech": "IFU",
                                        "dpr.type": "DISTORTION",
                                        })

wave_ifu_class = classification_rule("IFU_WAVE_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "WAVE",
                                  })

wave_cal_ifu_class = classification_rule("IFU_WAVECAL",
                                     {"pro.catg": "IFU_WAVECAL",
                                     })

rsrf_ifu_class = classification_rule("IFU_RSRF_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "RSRF",
                                 })

wcu_off_ifu_class = classification_rule("IFU_WCU_OFF_RAW",
                                        {"instrume": "METIS",
                                         "dpr.catg": "CALIB",
                                         "dpr.tech": "IFU",
                                         "dpr.type": "DARK,WCUOFF",
                                         })

rsrf_prod_ifu_class = classification_rule("RSRF_IFU",
                                      {"pro.catg": "RSRF_IFU",
                                       })

std_ifu_class = classification_rule("IFU_STD_RAW",
                                {"instrume": "METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "STD",
                                 })

sky_ifu_class = classification_rule("IFU_SKY_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.tech": "IFU",
                                     "dpr.type": "SKY",
                                     })

sci_ifu_class = classification_rule("IFU_SCI_RAW",
                                {"instrume": "METIS",
                                 "dpr.catg": "SCIENCE",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "OBJECT",
                                 })

persistence_class = classification_rule("PERSISTENCE_MAP",
                                        {"pro.catg": "PERSISTENCE_MAP",
                                         })

master_dark_ifu_class = classification_rule("MASTER_DARK_IFU",
                                        {"pro.catg": "MASTER_DARK_IFU",
                                         })

distortion_table_ifu_class = classification_rule("IFU_DISTORTION_TABLE",
                                             {"pro.catg": "IFU_DISTORTION_TABLE",
                                              })

calib_rsrf_ifu_class = classification_rule("RSRF_IFU",
                                       {"pro.catg": "RSRF_IFU",
                                        })

sci_comb_ifu_class = classification_rule("IFU_SCI_COMBINED",
                                {"pro.catg": "IFU_SCI_COMBINED",
                                 })

sci_reduce_ifu_class = classification_rule("IFU_SCI_REDUCED",
                                       {"pro.catg": "IFU_SCI_REDUCED",
                                        })

std_comb_ifu_class = classification_rule("IFU_STD_COMBINED",
                                     {"pro.catg": "IFU_STD_COMBINED",
                                      })

fluxstd_ifu_class = classification_rule("FLUXSTD_CATALOG",
                                    {"pro.catg": "FLUXSTD_CATALOG",
                                     })

lsf_kernel_class = classification_rule("LSF_KERNEL",
                                       {"pro.catg": "LSF_KERNEL",
                                        })

atm_profile_class = classification_rule("ATM_PROFILE",
                                        {"pro.catg": "ATM_PROFILE",
                                         })

telluric_ifu_class =classification_rule("IFU_TELLURIC",
                                        {"pro.catg": "IFU_TELLURIC",
                                         })

flux_tab_class = classification_rule("FLUXCAL_TAB",
                                     {"pro.catg": "FLUXCAL_TAB",
                                      })


# ----- LM LSS Classifications -----

# Slitloss files (TODO: Check the difference to STATIC ones - doubly defined??  Also check whether img or lss mode)
lm_slitloss_class = classification_rule("LM_ADC_SLITLOSS_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "SLITLOSS",
    })

# RSRF / FLATFIELDS 
lm_rsrf_raw_class = classification_rule("LM_LSS_RSRF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "FLAT,LAMP",
    })

# RSRF pinhole frames 
lm_rsrf_pinh_class = classification_rule("LM_LSS_RSRF_PINH_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "FLAT,LAMP,PINH",
    })

# Wavelength calib files (WCU laser sources)
lm_wave_class = classification_rule("LM_LSS_WAVE_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "WAVE",
    })

# Standard stars raw
lm_lss_raw_std_class = classification_rule("LM_LSS_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "STD",
    })

# Science observations in LSS mode
lm_lss_sci_raw_class = classification_rule("LM_LSS_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "LSS,LM",
    })

# MASTERDARK (master calibration)
MASTERDARK_class = classification_rule("MASTERDARK",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "MASTERDARK",
     metis_kwd.det_id: "2RG",
    })

# LM LSS final product (1D flux)
lm_lss_sci_flux_1d_class = classification_rule("LM_LSS_SCI_FLUX_1D",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "LM_LSS_SCI_FLUX_1D",
    })

# Static / external calibration products
# Atmospheric line catalogue
static_atm_line_cat_class = classification_rule("ATM_LINE_CAT",
    {metis_kwd.pro_catg: "ATM_LINE_CAT",
    })

# Table with WCU laser wavelengthes
static_laser_tab_class = classification_rule("LASER_TAB",
    {metis_kwd.pro_catg: "LASER_TAB",
    })

# Catalogue of standard stars
static_ref_std_cat_class = classification_rule("REF_STD_CAT",
    {metis_kwd.pro_catg: "REF_STD_CAT",
    })

# Distortion solution
static_lm_lss_dist_sol_class = classification_rule("LM_LSS_DIST_SOL",
    {metis_kwd.pro_catg: "LM_LSS_DIST_SOL",
    })

# First wavelength calibration guess
static_lm_lss_wave_guess_class = classification_rule("LM_LSS_WAVE_GUESS",
    {metis_kwd.pro_catg: "LM_LSS_WAVE_GUESS",
    })

# Static PSF model
static_ao_psf_model_class = classification_rule("AO_PSF_MODEL",
    {metis_kwd.pro_catg: "AO_PSF_MODEL",
    })

# ADC Slitloss file
static_lm_adc_slitloss_class = classification_rule("LM_ADC_SLITLOSS",
    {metis_kwd.pro_catg: "LM_ADC_SLITLOSS",
    })

# Static GAIN map
static_gain_map_h2rg_class = classification_rule("GAIN_MAP_2RG",
    {metis_kwd.pro_catg: "GAIN_MAP_2RG",
    })

# Linearity file
static_linearity_h2rg_class = classification_rule("LINEARITY_2RG",
    {metis_kwd.pro_catg: "LINEARITY_2RG",
    })

# Bad pixel map
static_badpix_map_h2rg_class = classification_rule("BADPIX_MAP_2RG",
    {metis_kwd.pro_catg: "BADPIX_MAP_2RG",
    })

# Synthetic transmission for the LM LSS mode
static_lm_lss_synth_trans_class = classification_rule("LM_LSS_SYNTH_TRANS",
    {metis_kwd.pro_catg: "LM_LSS_SYNTH_TRANS",
    })

# Table for best-fit molecfit parameters
mf_best_fit_tab_class = classification_rule("MF_BEST_FIT_TAB",
    {metis_kwd.pro_catg: "MF_BEST_FIT_TAB",
    })


# ----- N LSS Classifications -----

# Slitloss files (TODO: Check the difference to STATIC ones - doubly defined??  Also check whether img or lss mode)
n_slitloss_class = classification_rule("N_ADC_SLITLOSS_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "SLITLOSS",
    })

# RSRF / FLATFIELDS 
n_rsrf_raw_class = classification_rule("N_LSS_RSRF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "FLAT,LAMP",
    })

# RSRF pinhole frames 
n_rsrf_pinh_class = classification_rule("N_LSS_RSRF_PINH_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "FLAT,LAMP,PINH",
    })

# Wavelength calib files (WCU laser sources)
n_wave_class = classification_rule("N_LSS_WAVE_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "WAVE",
    })

# Standard stars raw 
n_lss_raw_std_class = classification_rule("N_LSS_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "STD",
    })

# Science observations raw
n_lss_sci_raw_class = classification_rule("N_LSS_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "LSS,N",
    })

# MASTERDARK for GEO detector
MASTERDARK_geo_class = classification_rule("MASTERDARK",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "MASTERDARK",
     metis_kwd.det_id: "GEO",
    })

# N LSS product
n_lss_sci_flux_1d_class = classification_rule("N_LSS_SCI_FLUX_1D",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "N_LSS_SCI_FLUX_1D",
    })

# Distortion solution 
static_n_lss_dist_sol_class = classification_rule("N_LSS_DIST_SOL",
    {metis_kwd.pro_catg: "N_LSS_DIST_SOL",
    })

# First wavelength calibration guess 
static_n_lss_wave_guess_class = classification_rule("N_LSS_WAVE_GUESS",
    {metis_kwd.pro_catg: "N_LSS_WAVE_GUESS",
    })

# Static ADC Slitloss 
static_n_adc_slitloss_class = classification_rule("N_ADC_SLITLOSS",
    {metis_kwd.pro_catg: "N_ADC_SLITLOSS",
    })

# Static gain map for GEO detector
static_gain_map_geo_class = classification_rule("GAIN_MAP_GEO",
    {metis_kwd.pro_catg: "GAIN_MAP_GEO",
    })

# Static linearity file for GEO detector
static_linearity_geo_class = classification_rule("LINEARITY_GEO",
    {metis_kwd.pro_catg: "LINEARITY_GEO",
    })

# Static bad pixel map for GEO detector
static_badpix_map_geo_class = classification_rule("BADPIX_MAP_GEO",
    {metis_kwd.pro_catg: "BADPIX_MAP_GEO",
    })

# Synthetic transmission for the N LSS mode
static_n_lss_synth_trans_class = classification_rule("N_LSS_SYNTH_TRANS",
    {metis_kwd.pro_catg: "N_LSS_SYNTH_TRANS",
    })
