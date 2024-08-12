"""Classification rules for metis_n_img_wkf.py"""
from edps import classification_rule

bpm_class = classification_rule("BADPIX_MAP_GEO",
                                {"instrume": "METIS",
                                 "pro.catg": "BADPIX_MAP_GEO"
                                 })

detlin_on_class = classification_rule("DETLIN_GEO_ON",
                                      {"instrume": "METIS",
                                       "dpr.catg": "CALIB",
                                       "dpr.type": "DETLIN",
                                       "dpr.tech": "IMAGE,N"
                                    })

detlin_off_class = classification_rule("DETLIN_GEO_OFF",
                                       {"instrume": "METIS",
                                        "dpr.catg": "CALIB",
                                        "dpr.type": "DETLIN",
                                        "dpr.tech": "IMAGE,N"
                                    })

linearity_class = classification_rule("LINEARITY_CUBE",
                                      {"instrume": "METIS",
                                       "pro.catg": "LINEARITY_CUBE"
                                       })

darkraw_class = classification_rule("DARK_GEO_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.type": "DARK",
                                     "dpr.tech": "IMAGE,N"
                                     })

flat_lamp_class = classification_rule("N_FLAT_LAMP_RAW",
                                      {"instrume": "METIS",
                                       "dpr.catg": "CALIB",
                                       "dpr.type": "FLAT,LAMP",
                                       "dpr.tech": "IMAGE,N"
                                       })

flat_twilight_class = classification_rule("N_FLAT_TWILIGHT_RAW",
                                          {"instrume": "METIS",
                                           "dpr.catg": "CALIB",
                                           "dpr.type": "FLAT,TWILIGHT",
                                           "dpr.tech": "IMAGE,N"
                                           })

sciraw_class = classification_rule("N_IMAGE_SCI_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "SCIENCE",
                                    "dpr.type": "OBJECT",
                                    "dpr.tech": "IMAGE,N"
                                    })

stdraw_class = classification_rule("N_IMAGE_STD_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "CALIB",
                                    "dpr.type": "STD",
                                    "dpr.tech": "IMAGE,N"
                                    })

fluxstdcat_class = classification_rule("FLUXSTD_CATALOG",
                                       {"pro.catg": "FLUXSTD_CATALOG"}
                                       )
