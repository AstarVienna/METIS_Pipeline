from edps import classification_rule
from . import metis_keywords as metis_kwd

# Detector linearity calibration classification
detlin_2rg_raw_class = classification_rule("DETLIN_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DETLIN",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

# Dark frame calibration classification
rawdark_2rg_class = classification_rule("DARK_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

lm_distortion_raw_class = classification_rule("LM_DISTORTION_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DISTORTION",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

lm_wcu_off_raw_class = classification_rule("LM_WCU_OFF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK,WCUOFF",
     metis_kwd.drp_tech: "IMAGE,LM",
    })


# Lamp flat calibration classification
lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

# Twilight flat calibration classification
lm_twilightflat_class = classification_rule("LM_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

# Science observation classification
lm_raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

lm_raw_sky_class = classification_rule("LM_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

lm_raw_std_class = classification_rule("LM_IMAGE_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "STD",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

# ------- N BAND CLASSIFICATIONS -----------


# Detector linearity calibration classification
detlin_geo_raw_class = classification_rule("DETLIN_GEO_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DETLIN",
     metis_kwd.drp_tech: "IMAGE,N",
    })

# Dark frame calibration classification
rawdark_geo_class = classification_rule("DARK_GEO_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK",
     metis_kwd.drp_tech: "IMAGE,N",
    })

n_distortion_raw_class = classification_rule("N_DISTORTION_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DISTORTION",
     metis_kwd.drp_tech: "IMAGE,N",
    })

n_wcu_off_raw_class = classification_rule("N_WCU_OFF_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK,WCUOFF",
     metis_kwd.drp_tech: "IMAGE,N",
    })


# Lamp flat calibration classification
n_lampflat_class = classification_rule("N_FLAT_LAMP_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,LAMP",
     metis_kwd.drp_tech: "IMAGE,N",
    })

# Twilight flat calibration classification
n_twilightflat_class = classification_rule("N_TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.drp_tech: "IMAGE,N",
    })

# Science observation classification
n_raw_science_class = classification_rule("N_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.drp_tech: "IMAGE,N",
    })

n_raw_sky_class = classification_rule("N_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.drp_tech: "IMAGE,N",
    })

n_raw_std_class = classification_rule("N_IMAGE_STD_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "STD",
     metis_kwd.drp_tech: "IMAGE,N",
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
