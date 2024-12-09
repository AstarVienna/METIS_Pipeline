from edps import classification_rule

from . import metis_keywords as metis_kwd

detlin_class = classification_rule("DETLIN_2RG_RAW",
                                    {metis_kwd.instrume:"METIS", 
                                     metis_kwd.dpr_catg: "CALIB", 
                                     "dpr.type":"DETLIN",
                                     "dpr.tech":"IMAGE,LM",
                                     })

rawdark_class = classification_rule("DARK_2RG_RAW",
                                    {metis_kwd.instrume:"METIS", 
                                     "dpr.catg": "CALIB", 
                                     "dpr.type":"DARK",
                                     "dpr.tech":"IMAGE,LM",
                                     })

lm_lampflat_class = classification_rule("LM_FLAT_LAMP_RAW",
                                {metis_kwd.instrume:"METIS", 
                                 "dpr.catg":"CALIB", 
                                 "dpr.type":"FLAT,LAMP",
                                 "dpr.tech":"IMAGE,LM",
                                 })

twlightflat_class = classification_rule("TWLIGHT_FLAT",
                                {metis_kwd.instrume:"METIS", 
                                 "dpr.catg": "CALIB", 
                                 "dpr.type":"FLAT,TWILIGHT",
                                 "dpr.tech":"IMAGE,LM",
                                 })


raw_science_class = classification_rule("LM_IMAGE_SCI_RAW",
                                {metis_kwd.instrume:"METIS", 
                                 "dpr.catg":"SCIENCE", 
                                 "dpr.type":"OBJECT",
                                 "dpr.tech":"IMAGE,LM",
                                 })