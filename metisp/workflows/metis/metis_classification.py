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
dark_2rg_raw_class = classification_rule("DARK_2RG_RAW",
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
lm_flat_lamp_raw_class = classification_rule("LM_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# Twilight flat calibration classification
lm_twilight_flat_class = classification_rule("LM_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

# Science observation classification
lm_image_sci_raw_class = classification_rule("LM_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_image_sky_raw_class = classification_rule("LM_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.dpr_tech: "IMAGE,LM",
    })

lm_image_std_raw_class = classification_rule("LM_IMAGE_STD_RAW",
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
dark_geo_raw_class = classification_rule("DARK_GEO_RAW",
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
n_flat_lamp_raw_class = classification_rule("N_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

# Twilight flat calibration classification
n_twilight_flat_class = classification_rule("N_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

# Science observation classification
n_image_sci_raw_class = classification_rule("N_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_image_sky_raw_class = classification_rule("N_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.dpr_tech: "IMAGE,N",
    })

n_image_std_raw_class = classification_rule("N_IMAGE_STD_RAW",
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

badpix_map_ifu_class = classification_rule("BADPIX_MAP_IFU",
                                   {"pro.catg": "BADPIX_MAP_det",
                                    })

detlin_ifu_raw_class = classification_rule("DETLIN_IFU_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "CALIB",
                                    "dpr.type": "DETLIN",
                                    "dpr.tech": "IFU",                                    
                                    })

gain_map_ifu_class = classification_rule("GAIN_MAP_IFU",
                                     {"pro.catg": "GAIN_MAP_det",
                                      })

linearity_ifu_class = classification_rule("LINEARITY_IFU",
                                    {"pro.catg": "LINEARITY_det",
                                    })

dark_ifu_raw_class = classification_rule("DARK_IFU_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.tech": "IFU",
                                     "dpr.type": "DARK",
                                     })

ifu_distortion_raw_class = classification_rule("IFU_DISTORTION_RAW",
                                       {"instrume": "METIS",
                                        "dpr.catg": "CALIB",
                                        "dpr.tech": "IFU",
                                        "dpr.type": "DISTORTION",
                                        })

ifu_wave_raw_class = classification_rule("IFU_WAVE_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "WAVE",
                                  })

ifu_wavecal_class = classification_rule("IFU_WAVECAL",
                                     {"pro.catg": "IFU_WAVECAL",
                                     })

ifu_rsrf_raw_class = classification_rule("IFU_RSRF_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "RSRF",
                                 })

ifu_wcu_off_raw_class = classification_rule("IFU_WCU_OFF_RAW",
                                        {"instrume": "METIS",
                                         "dpr.catg": "CALIB",
                                         "dpr.tech": "IFU",
                                         "dpr.type": "DARK,WCUOFF",
                                         })

rsrf_ifu_class = classification_rule("RSRF_IFU",
                                      {"pro.catg": "RSRF_IFU",
                                       })

ifu_std_raw_class = classification_rule("IFU_STD_RAW",
                                {"instrume": "METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "STD",
                                 })

ifu_sky_raw_class = classification_rule("IFU_SKY_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.tech": "IFU",
                                     "dpr.type": "SKY",
                                     })

ifu_sci_raw_class = classification_rule("IFU_SCI_RAW",
                                {"instrume": "METIS",
                                 "dpr.catg": "SCIENCE",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "OBJECT",
                                 })

persistence_map_class = classification_rule("PERSISTENCE_MAP",
                                        {"pro.catg": "PERSISTENCE_MAP",
                                         })

master_dark_ifu_class = classification_rule("MASTER_DARK_IFU",
                                        {"pro.catg": "MASTER_DARK_IFU",
                                         })

ifu_distortion_table_class = classification_rule("IFU_DISTORTION_TABLE",
                                             {"pro.catg": "IFU_DISTORTION_TABLE",
                                              })


ifu_sci_combined_class = classification_rule("IFU_SCI_COMBINED",
                                {"pro.catg": "IFU_SCI_COMBINED",
                                 })

ifu_sci_reduced_class = classification_rule("IFU_SCI_REDUCED",
                                       {"pro.catg": "IFU_SCI_REDUCED",
                                        })

ifu_std_combined_class = classification_rule("IFU_STD_COMBINED",
                                     {"pro.catg": "IFU_STD_COMBINED",
                                      })

lsf_kernel_class = classification_rule("LSF_KERNEL",
                                       {"pro.catg": "LSF_KERNEL",
                                        })

atm_profile_class = classification_rule("ATM_PROFILE",
                                        {"pro.catg": "ATM_PROFILE",
                                         })

ifu_telluric_class =classification_rule("IFU_TELLURIC",
                                        {"pro.catg": "IFU_TELLURIC",
                                         })

flux_tab_class = classification_rule("FLUXCAL_TAB",
                                     {"pro.catg": "FLUXCAL_TAB",
                                      })


# ----- LM LSS Classifications -----

# Slitloss files (TODO: Check the difference to STATIC ones - doubly defined??  Also check whether img or lss mode)
lm_adc_slitloss_raw_class = classification_rule("LM_ADC_SLITLOSS_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "SLITLOSS",
    })

# RSRF / FLATFIELDS 
lm_lss_rsrf_raw_class = classification_rule("LM_LSS_RSRF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "FLAT,LAMP",
    })

# RSRF pinhole frames 
lm_lss_rsrf_pinh_raw_class = classification_rule("LM_LSS_RSRF_PINH_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "FLAT,LAMP,PINH",
    })

# Wavelength calib files (WCU laser sources)
lm_lss_wave_raw_class = classification_rule("LM_LSS_WAVE_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,LM",
     metis_kwd.dpr_type: "WAVE",
    })

# Standard stars raw
lm_lss_std_raw_class = classification_rule("LM_LSS_STD_RAW",
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

# LM LSS final product (1D flux)
lm_lss_sci_flux_1d_class = classification_rule("LM_LSS_SCI_FLUX_1D",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "LM_LSS_SCI_FLUX_1D",
    })

# Static / external calibration products
# Atmospheric line catalogue
atm_line_cat_class = classification_rule("ATM_LINE_CAT",
    {metis_kwd.pro_catg: "ATM_LINE_CAT",
    })

# Table with WCU laser wavelengthes
laser_tab_class = classification_rule("LASER_TAB",
    {metis_kwd.pro_catg: "LASER_TAB",
    })

# Catalogue of standard stars
ref_std_cat_class = classification_rule("REF_STD_CAT",
    {metis_kwd.pro_catg: "REF_STD_CAT",
    })

# Distortion solution
lm_lss_dist_sol_class = classification_rule("LM_LSS_DIST_SOL",
    {metis_kwd.pro_catg: "LM_LSS_DIST_SOL",
    })

# First wavelength calibration guess
lm_lss_wave_guess_class = classification_rule("LM_LSS_WAVE_GUESS",
    {metis_kwd.pro_catg: "LM_LSS_WAVE_GUESS",
    })

# Static PSF model
ao_psf_model_class = classification_rule("AO_PSF_MODEL",
    {metis_kwd.pro_catg: "AO_PSF_MODEL",
    })

# ADC Slitloss file
lm_adc_slitloss_class = classification_rule("LM_ADC_SLITLOSS",
    {metis_kwd.pro_catg: "LM_ADC_SLITLOSS",
    })

# Static GAIN map
gain_map_h2rg_class = classification_rule("GAIN_MAP_2RG",
    {metis_kwd.pro_catg: "GAIN_MAP_det",
    })

# Linearity file
linearity_h2rg_class = classification_rule("LINEARITY_2RG",
    {metis_kwd.pro_catg: "LINEARITY_det",
    })

# Bad pixel map
badpix_map_h2rg_class = classification_rule("BADPIX_MAP_2RG",
    {metis_kwd.pro_catg: "BADPIX_MAP_det",
    })

# Synthetic transmission for the LM LSS mode
lm_lss_synth_trans_class = classification_rule("LM_LSS_SYNTH_TRANS",
    {metis_kwd.pro_catg: "LM_LSS_SYNTH_TRANS",
    })

# Table for best-fit molecfit parameters
mf_best_fit_tab_class = classification_rule("MF_BEST_FIT_TAB",
    {metis_kwd.pro_catg: "MF_BEST_FIT_TAB",
    })


# ----- N LSS Classifications -----

# Slitloss files (TODO: Check the difference to STATIC ones - doubly defined??  Also check whether img or lss mode)
n_adc_slitloss_raw_class = classification_rule("N_ADC_SLITLOSS_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "SLITLOSS",
    })

# RSRF / FLATFIELDS 
n_lss_rsrf_raw_class = classification_rule("N_LSS_RSRF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "FLAT,LAMP",
    })

# RSRF pinhole frames 
n_lss_rsrf_pinh_raw_class = classification_rule("N_LSS_RSRF_PINH_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "FLAT,LAMP,PINH",
    })

# Wavelength calib files (WCU laser sources)
n_lss_wave_raw_class = classification_rule("N_LSS_WAVE_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_tech: "LSS,N",
     metis_kwd.dpr_type: "WAVE",
    })

# Standard stars raw 
n_lss_std_raw_class = classification_rule("N_LSS_STD_RAW",
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

# N LSS product
n_lss_sci_flux_1d_class = classification_rule("N_LSS_SCI_FLUX_1D",
    {metis_kwd.instrume: "METIS",
     metis_kwd.pro_catg: "N_LSS_SCI_FLUX_1D",
    })

# Distortion solution 
n_lss_dist_sol_class = classification_rule("N_LSS_DIST_SOL",
    {metis_kwd.pro_catg: "N_LSS_DIST_SOL",
    })

# First wavelength calibration guess 
n_lss_wave_guess_class = classification_rule("N_LSS_WAVE_GUESS",
    {metis_kwd.pro_catg: "N_LSS_WAVE_GUESS",
    })

# Static ADC Slitloss 
n_adc_slitloss_class = classification_rule("N_ADC_SLITLOSS",
    {metis_kwd.pro_catg: "N_ADC_SLITLOSS",
    })

# Static gain map for GEO detector
gain_map_geo_class = classification_rule("GAIN_MAP_GEO",
    {metis_kwd.pro_catg: "GAIN_MAP_GEO",
    })

# Static linearity file for GEO detector
linearity_geo_class = classification_rule("LINEARITY_GEO",
    {metis_kwd.pro_catg: "LINEARITY_GEO",
    })

# Static bad pixel map for GEO detector
badpix_map_geo_class = classification_rule("BADPIX_MAP_GEO",
    {metis_kwd.pro_catg: "BADPIX_MAP_GEO",
    })

# Synthetic transmission for the N LSS mode
n_lss_synth_trans_class = classification_rule("N_LSS_SYNTH_TRANS",
    {metis_kwd.pro_catg: "N_LSS_SYNTH_TRANS",
    })
