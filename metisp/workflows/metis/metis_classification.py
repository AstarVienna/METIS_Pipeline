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
rawdark_class = classification_rule("DARK_2RG_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "DARK",
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
twilightflat_class = classification_rule("TWILIGHT_FLAT",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "CALIB",
     metis_kwd.dpr_type: "FLAT,TWILIGHT",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

# Science observation classification
raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "OBJECT",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

raw_sky_class = classification_rule("LM_IMAGE_SKY_RAW",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "SCIENCE",
     metis_kwd.dpr_type: "SKY",
     metis_kwd.drp_tech: "IMAGE,LM",
    })

lm_sci_basic_reduced_class = classification_rule("LM_SCI_BASIC_REDUCED",
    {metis_kwd.instrume: "METIS",
     metis_kwd.dpr_catg: "LM_SCI_BASIC_REDUCED",
     metis_kwd.dpr_type: "REDUCED",
     metis_kwd.drp_tech: "IMAGE,LM",
    })
