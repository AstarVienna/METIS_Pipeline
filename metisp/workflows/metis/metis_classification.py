from edps import classification_rule
from .metis_keywords import *

# Detector linearity calibration classification
detlin_2rg_raw_class = classification_rule("DETLIN_2RG_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DETLIN",
     dpr_tech: "IMAGE,LM",
    })

# Dark frame calibration classification
rawdark_2rg_class = classification_rule("DARK_2RG_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DARK",
     dpr_tech: "IMAGE,LM",
    })

lm_distortion_raw_class = classification_rule("LM_DISTORTION_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DISTORTION",
     dpr_tech: "IMAGE,LM",
    })

lm_wcu_off_raw_class = classification_rule("LM_WCU_OFF_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DARK,WCUOFF",
     dpr_tech: "IMAGE,LM",
    })


# Lamp flat calibration classification
lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "FLAT,LAMP",
     dpr_tech: "IMAGE,LM",
    })

# Twilight flat calibration classification
lm_twilightflat_class = classification_rule("LM_TWILIGHT_FLAT",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "FLAT,TWILIGHT",
     dpr_tech: "IMAGE,LM",
    })

# Science observation classification
lm_raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
    {instrume: "METIS",
     dpr_catg: "SCIENCE",
     dpr_type: "OBJECT",
     dpr_tech: "IMAGE,LM",
    })

lm_raw_sky_class = classification_rule("LM_IMAGE_SKY_RAW",
    {instrume: "METIS",
     dpr_catg: "SCIENCE",
     dpr_type: "SKY",
     dpr_tech: "IMAGE,LM",
    })

lm_raw_std_class = classification_rule("LM_IMAGE_STD_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "STD",
     dpr_tech: "IMAGE,LM",
    })

# ------- N BAND CLASSIFICATIONS -----------


# Detector linearity calibration classification
detlin_geo_raw_class = classification_rule("DETLIN_GEO_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DETLIN",
     dpr_tech: "IMAGE,N",
    })

# Dark frame calibration classification
rawdark_geo_class = classification_rule("DARK_GEO_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DARK",
     dpr_tech: "IMAGE,N",
    })

n_distortion_raw_class = classification_rule("N_DISTORTION_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DISTORTION",
     dpr_tech: "IMAGE,N",
    })

n_wcu_off_raw_class = classification_rule("N_WCU_OFF_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "DARK,WCUOFF",
     dpr_tech: "IMAGE,N",
    })


# Lamp flat calibration classification
n_lampflat_class = classification_rule("N_FLAT_LAMP_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "FLAT,LAMP",
     dpr_tech: "IMAGE,N",
    })

# Twilight flat calibration classification
n_twilightflat_class = classification_rule("N_TWILIGHT_FLAT",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "FLAT,TWILIGHT",
     dpr_tech: "IMAGE,N",
    })

# Science observation classification
n_raw_science_class = classification_rule("N_IMAGE_SCI_RAW",
    {instrume: "METIS",
     dpr_catg: "SCIENCE",
     dpr_type: "OBJECT",
     dpr_tech: "IMAGE,N",
    })

n_raw_sky_class = classification_rule("N_IMAGE_SKY_RAW",
    {instrume: "METIS",
     dpr_catg: "SCIENCE",
     dpr_type: "SKY",
     dpr_tech: "IMAGE,N",
    })

n_raw_std_class = classification_rule("N_IMAGE_STD_RAW",
    {instrume: "METIS",
     dpr_catg: "CALIB",
     dpr_type: "STD",
     dpr_tech: "IMAGE,N",
    })



# Flux standard catalog classification
fluxstd_catalog_class = classification_rule("FLUXSTD_CATALOG",
     {pro_catg: "FLUXSTD_CATALOG",
    })

# Pinhole table classification
pinhole_table_class = classification_rule("PINHOLE_TABLE",
    {pro_catg: "PINHOLE_TABLE",
    })

# --- IFU Classifications ---

badpix_ifu_class = classification_rule("BADPIX_MAP_IFU",
                                   {pro_catg: "BADPIX_MAP_det",
                                    })

detlin_ifu_class = classification_rule("DETLIN_IFU_RAW",
                                   {instrume: "METIS",
                                    dpr_catg: "CALIB",
                                    dpr_type: "DETLIN",
                                    dpr_tech: "IFU",                                    
                                    })

gain_map_ifu_class = classification_rule("GAIN_MAP_IFU",
                                     {pro_catg: "GAIN_MAP_det",
                                      })

linearity_det_ifu_class = classification_rule("LINEARITY_IFU",
                                    {pro_catg: "LINEARITY_det",
                                    })

rawdark_ifu_class = classification_rule("DARK_IFU_RAW",
                                    {instrume: "METIS",
                                     dpr_catg: "CALIB",
                                     dpr_tech: "IFU",
                                     dpr_type: "DARK",
                                     })

distortion_ifu_class = classification_rule("IFU_DISTORTION_RAW",
                                       {instrume: "METIS",
                                        dpr_catg: "CALIB",
                                        dpr_tech: "IFU",
                                        dpr_type: "DISTORTION",
                                        })

wave_ifu_class = classification_rule("IFU_WAVE_RAW",
                                 {instrume: "METIS",
                                  dpr_catg: "CALIB",
                                  dpr_tech: "IFU",
                                  dpr_type: "WAVE",
                                  })

wave_cal_ifu_class = classification_rule("IFU_WAVECAL",
                                     {pro_catg: "IFU_WAVECAL",
                                     })

rsrf_ifu_class = classification_rule("IFU_RSRF_RAW",
                                 {instrume: "METIS",
                                  dpr_catg: "CALIB",
                                  dpr_tech: "IFU",
                                  dpr_type: "RSRF",
                                 })

wcu_off_ifu_class = classification_rule("IFU_WCU_OFF_RAW",
                                        {instrume: "METIS",
                                         dpr_catg: "CALIB",
                                         dpr_tech: "IFU",
                                         dpr_type: "DARK,WCUOFF",
                                         })

rsrf_prod_ifu_class = classification_rule("RSRF_IFU",
                                      {pro_catg: "RSRF_IFU",
                                       })

std_ifu_class = classification_rule("IFU_STD_RAW",
                                {instrume: "METIS",
                                 dpr_catg: "CALIB",
                                 dpr_tech: "IFU",
                                 dpr_type: "STD",
                                 })

sky_ifu_class = classification_rule("IFU_SKY_RAW",
                                    {instrume: "METIS",
                                     dpr_catg: "CALIB",
                                     dpr_tech: "IFU",
                                     dpr_type: "SKY",
                                     })

sci_ifu_class = classification_rule("IFU_SCI_RAW",
                                {instrume: "METIS",
                                 dpr_catg: "SCIENCE",
                                 dpr_tech: "IFU",
                                 dpr_type: "OBJECT",
                                 })

persistence_class = classification_rule("PERSISTENCE_MAP",
                                        {pro_catg: "PERSISTENCE_MAP",
                                         })

master_dark_ifu_class = classification_rule("MASTER_DARK_IFU",
                                        {pro_catg: "MASTER_DARK_IFU",
                                         })

distortion_table_ifu_class = classification_rule("IFU_DISTORTION_TABLE",
                                             {pro_catg: "IFU_DISTORTION_TABLE",
                                              })

calib_rsrf_ifu_class = classification_rule("RSRF_IFU",
                                       {pro_catg: "RSRF_IFU",
                                        })

sci_comb_ifu_class = classification_rule("IFU_SCI_COMBINED",
                                {pro_catg: "IFU_SCI_COMBINED",
                                 })

sci_reduce_ifu_class = classification_rule("IFU_SCI_REDUCED",
                                       {pro_catg: "IFU_SCI_REDUCED",
                                        })

std_comb_ifu_class = classification_rule("IFU_STD_COMBINED",
                                     {pro_catg: "IFU_STD_COMBINED",
                                      })

fluxstd_ifu_class = classification_rule("FLUXSTD_CATALOG",
                                    {pro_catg: "FLUXSTD_CATALOG",
                                     })

lsf_kernel_class = classification_rule("LSF_KERNEL",
                                       {pro_catg: "LSF_KERNEL",
                                        })

atm_profile_class = classification_rule("ATM_PROFILE",
                                        {pro_catg: "ATM_PROFILE",
                                         })

telluric_ifu_class =classification_rule("IFU_TELLURIC",
                                        {pro_catg: "IFU_TELLURIC",
                                         })

flux_tab_class = classification_rule("FLUXCAL_TAB",
                                     {pro_catg: "FLUXCAL_TAB",
                                      })
