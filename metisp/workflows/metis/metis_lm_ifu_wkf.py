from edps import task, data_source, classification_rule, SCIENCE

# --- Classification Rules ---

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

lin_det_ifu_class = classification_rule("LINEARITY_IFU",
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

pinhole_class = classification_rule("PINHOLE_TABLE",
                                    {"pro.catg": "PINHOLE_TABLE",
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

telluric_ifu_class =classification_rule("IFU_TELLURIC")

flux_tab_class = classification_rule("FLUXCAL_TAB")

# --- Data sources ---

bad_pix_ifu_calib = (data_source()
                 .with_classification_rule(badpix_ifu_class)
                 .build())

detlin_ifu_raw = (data_source()
              .with_classification_rule(detlin_ifu_class)
              .with_match_keywords(["instrume"])
              .build())

dark_ifu_raw = (data_source()
            .with_classification_rule(rawdark_ifu_class)
            .with_match_keywords(["instrume"])
            .build())

distortion_ifu_raw = (data_source()
                  .with_classification_rule(distortion_ifu_class)
                  .with_match_keywords(["instrume"])
                  .build())

wave_ifu_raw = (data_source()
            .with_classification_rule(wave_ifu_class)
            .with_match_keywords(["instrume"])
            .build())

rsrf_ifu_raw = (data_source()
            .with_classification_rule(rsrf_ifu_class)
            .with_match_keywords(["instrume"])
            .build())

wcu_off_ifu_raw = (data_source()
                   .with_classification_rule(wcu_off_ifu_class)
                   .with_match_keywords(["instrume"])
                   .build())

std_ifu_raw = (data_source()
           .with_classification_rule(std_ifu_class)
           .with_match_keywords(["instrume"])
           .build())

sky_ifu_raw = (data_source()
               .with_classification_rule(sky_ifu_class)
               .with_match_keywords(["instrume"])
               .build())

sci_ifu_raw = (data_source()
           .with_classification_rule(sci_ifu_class)
           .with_match_keywords(["instrume"])
           .build())

calib_persistence = (data_source()
                     .with_classification_rule(persistence_class)
                     .with_match_keywords(["simple"])
                     .build())

calib_pinhole = (data_source()
                 .with_classification_rule(pinhole_class)
                 .with_match_keywords(["simple"])
                 .build())

calib_lsf_kernel = (data_source()
                    .with_classification_rule(lsf_kernel_class)
                    .with_match_keywords(["simple"])
                    .build())

calib_flux_std = (data_source()
                  .with_classification_rule(fluxstd_ifu_class)
                  .with_match_keywords(["simple"])
                  .build())

calib_atm_profile = (data_source()
                     .with_classification_rule(atm_profile_class)
                     .with_match_keywords(["simple"])
                     .build())

# --- Processing tasks ---


lingain_ifu_task = (task("metis_ifu_lingain")
                .with_recipe("metis_det_lingain")
                .with_main_input(detlin_ifu_raw)
                .with_associated_input(bad_pix_ifu_calib, min_ret=0)
                .build())

dark_ifu_task = (task("metis_ifu_dark")
             .with_recipe("metis_det_dark")
             .with_main_input(dark_ifu_raw)
             .with_associated_input(bad_pix_ifu_calib, min_ret=0)
             .with_associated_input(lingain_ifu_task)
             .with_associated_input(calib_persistence, min_ret=0)
             .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, persistence_class)
             .build())

distortion_ifu_task = (task("metis_ifu_distortion")
                   .with_recipe("metis_ifu_distortion")
                   .with_main_input(distortion_ifu_raw)
                   .with_associated_input(bad_pix_ifu_calib, min_ret=0)
                   .with_associated_input(lingain_ifu_task)
                   .with_associated_input(calib_persistence, min_ret=0)
                   .with_associated_input(calib_pinhole)
                   .with_associated_input(dark_ifu_task)
                   .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, master_dark_ifu_class, persistence_class, pinhole_class)
                   .build())

wave_ifu_task = (task("metis_ifu_wavecal")
             .with_recipe("metis_ifu_wavecal")
             .with_main_input(wave_ifu_raw)
             .with_associated_input(bad_pix_ifu_calib, min_ret=0)
             .with_associated_input(lingain_ifu_task)
             .with_associated_input(calib_persistence, min_ret=0)
             .with_associated_input(dark_ifu_task)
             .with_associated_input(distortion_ifu_task)
             .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, master_dark_ifu_class, distortion_table_ifu_class, persistence_class, pinhole_class)
             .build())

rsrf_ifu_task = (task("metis_ifu_rsrf")
             .with_recipe("metis_ifu_rsrf")
             .with_main_input(rsrf_ifu_raw)
             .with_associated_input(bad_pix_ifu_calib, min_ret=0)
             .with_associated_input(lingain_ifu_task)
             .with_associated_input(calib_persistence, min_ret=0)
             .with_associated_input(dark_ifu_task)
             .with_associated_input(distortion_ifu_task)
             .with_associated_input(wcu_off_ifu_raw)
             .with_associated_input(wave_ifu_task)
             .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, master_dark_ifu_class, distortion_table_ifu_class, wave_cal_ifu_class, persistence_class)
             .build())

std_ifu_task = (task("metis_std_reduce")
            .with_recipe("metis_ifu_reduce")
            .with_main_input(std_ifu_raw)
            .with_associated_input(bad_pix_ifu_calib, min_ret=0)
            .with_associated_input(lingain_ifu_task)
            .with_associated_input(calib_persistence, min_ret=0)
            .with_associated_input(dark_ifu_task)
            .with_associated_input(distortion_ifu_task)
            .with_associated_input(wave_ifu_task)
            .with_associated_input(rsrf_ifu_task)
            .with_associated_input(sky_ifu_raw)
            .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, master_dark_ifu_class, persistence_class, distortion_table_ifu_class, wave_cal_ifu_class, rsrf_prod_ifu_class)
            .build())

sci_ifu_task = (task("metis_ifu_sci_reduce")
            .with_recipe("metis_ifu_reduce")
            .with_main_input(sci_ifu_raw)
            .with_associated_input(bad_pix_ifu_calib, min_ret=0)
            .with_associated_input(lingain_ifu_task)
            .with_associated_input(calib_persistence, min_ret=0)
            .with_associated_input(dark_ifu_task)
            .with_associated_input(distortion_ifu_task)
            .with_associated_input(wave_ifu_task)
            .with_associated_input(rsrf_ifu_task)
            .with_associated_input(sky_ifu_raw)
            .with_input_filter(lin_det_ifu_class, gain_map_ifu_class, master_dark_ifu_class, persistence_class, distortion_table_ifu_class, wave_cal_ifu_class, rsrf_prod_ifu_class)
            .build())

telluric_sci_ifu_task = (task("metis_ifu_sci_telluric")
                .with_recipe("metis_ifu_telluric")
                .with_main_input(sci_ifu_task)
                .with_associated_input(calib_flux_std)
                .with_associated_input(calib_lsf_kernel)
                .with_associated_input(calib_atm_profile)
                .with_input_filter(sci_reduce_ifu_class, sci_comb_ifu_class, fluxstd_ifu_class, lsf_kernel_class, atm_profile_class)
                .with_output_filter(telluric_ifu_class)
                .build())

telluric_std_ifu_task = (task("metis_ifu_std_telluric")
                 .with_recipe("metis_ifu_telluric")
                 .with_main_input(std_ifu_task)
                 .with_associated_input(calib_flux_std)
                 .with_associated_input(calib_lsf_kernel)
                 .with_associated_input(calib_atm_profile)
                 .with_input_filter(std_comb_ifu_class, fluxstd_ifu_class, lsf_kernel_class, atm_profile_class)
                 .with_output_filter(flux_tab_class)
                 .build())

calibrate_ifu_task = (task("metis_ifu_calibrate")
                  .with_recipe("metis_ifu_calibrate")
                  .with_main_input(sci_ifu_task)
                  .with_associated_input(telluric_sci_ifu_task)
                  .with_associated_input(telluric_std_ifu_task)
                  .with_input_filter(sci_reduce_ifu_class, flux_tab_class, telluric_ifu_class)
                  .build())

post_process_ifu_task = (task("metis_ifu_postprocess")
                     .with_recipe("metis_ifu_postprocess")
                     .with_main_input(calibrate_ifu_task)
                     .with_meta_targets([SCIENCE])
                     .build())