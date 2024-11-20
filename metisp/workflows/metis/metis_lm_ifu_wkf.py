from edps import task, data_source, classification_rule

# --- Classification Rules ---
detlin_class = classification_rule("DETLIN_IFU_RAW",
                                   {"instrume": "METIS",
                                    "dpr.catg": "CALIB",
                                    "dpr.type": "DETLIN",
                                    "dpr.tech": "IFU",                                    
                                    })

lin_det_class = classification_rule("LINEARITY_det",
                                    {"pro.catg": "LINEARITY_det",
                                    })

rawdark_class = classification_rule("DARK_IFU_RAW",
                                    {"instrume": "METIS",
                                     "dpr.catg": "CALIB",
                                     "dpr.tech": "IFU",
                                     "dpr.type": "DARK",
                                     })

distortion_class = classification_rule("IFU_DISTORTION_RAW",
                                       {"instrume": "METIS",
                                        "dpr.catg": "CALIB",
                                        "dpr.tech": "IFU",
                                        "dipr.type": "DISTORTION",
                                        })

wave_class = classification_rule("IFU_WAVE_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "WAVE",
                                  })

rsrf_class = classification_rule("IFU_RSRF_RAW",
                                 {"instrume": "METIS",
                                  "dpr.catg": "CALIB",
                                  "dpr.tech": "IFU",
                                  "dpr.type": "RSRF",
                                 })

std_class = classification_rule("IFU_STD_RAW",
                                {"insturme": "METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "STD",
                                 })

sci_class = classification_rule("IFU_SCI_RAW",
                                {"instrume": "METIS",
                                 "dpr.catg": "CALIB",
                                 "dpr.tech": "IFU",
                                 "dpr.type": "OBJECT",
                                 })

persistence_class = classification_rule("PERSISTENCE_MAP",
                                        {"pro.catg": "PERSISTENCE_MAP",
                                         })

pinhole_class = classification_rule("PINHOLE_TABLE",
                                    {"pro.catg": "PINHOLE_TABLE"})

master_dark_class = classification_rule("MASTER_DARK_IFU",
                                        {"pro.catg": "MASTER_DARK_IFU",
                                         })

distortion_table_class = classification_rule("IFU_DISTORTION_TABLE",
                                             {"pro.catg": "IFU_DISTORTION_TABLE",
                                              })

calib_rsrf_class = classification_rule("RSRF_IFU",
                                       {"pro.catg": "RSRF_IFU",
                                        })

sci_comb_class = classification_rule("IFU_SCI_COMBINED",
                                {"pro.catg": "IFU_SCI_COMBINED",
                                 })

std_comb_class = classification_rule("IFU_STD_COMBINED",
                                     {"pro.catg": "IFU_STD_COMBINED",
                                      })

fluxstd_class = classification_rule("FLUXSTD_CATALOG",
                                    {"pro.catg": "FLUXSTD_CATALOG",
                                     })

lsf_kernel_class = classification_rule("LSF_KERNEL",
                                       {"pro.catg": "LSF_KERNEL",
                                        })

atm_profile_class = classification_rule("ATM_PROFILE",
                                        {"pro.catg": "ATM_PROFILE",
                                         })

sci_telluric_class =classification_rule("IFU_TELLURIC",
                                        {"pro.catg": "IFU_TELLURIC"})

# --- Data sources ---
detlin_raw = (data_source()
              .with_classification_rule(detlin_class)
              .with_match_keywords(["instrume"])
              .build())

dark_raw = (data_source()
            .with_classification_rule(rawdark_class)
            .with_match_keywords(["instrume"])
            .build())

raw_distortion = (data_source()
                  .with_classification_rule(rawdark_class)
                  .with_match_keywords(["instrume"])
                  .build())

raw_wave = (data_source()
            .with_classification_rule(wave_class)
            .with_match_keywords(["instrume"])
            .build())

raw_rsrf = (data_source()
            .with_classification_rule(rsrf_class)
            .with_match_keywords(["instrume"])
            .build())

raw_std = (data_source()
           .with_classification_rule(std_class)
           .with_match_keywords(["instrume"])
           .build())

raw_sci = (data_source()
           .with_classification_rule(sci_class)
           .with_match_keywords(["instrume"])
           .build())

calib_persistence = (data_source()
                     .with_classification_rule(persistence_class)
                     .build())

calib_pinhole = (data_source()
                 .with_classification_rule(pinhole_class)
                 .build())

calib_lsf_kernel = (data_source()
                    .with_classification_rule(lsf_kernel_class)
                    .build())

calib_flux_std = (data_source()
                  .with_classification_rule(fluxstd_class)
                  .build())

calib_atm_profile = (data_source()
                     .with_classification_rule(atm_profile_class)
                     .build())

# --- Processing tasks ---


lingain_task = (task("metis_ifu_lingain")
                .with_main_input(detlin_raw)
                .with_recipe("metis_det_lingain")
                .build())

dark_task = (task("metis_ifu_dark")
             .with_main_input(dark_raw)
             .with_associated_input(lingain_task)
             .with_input_filter(lin_det_class)
             .with_associated_input(calib_persistence)
             .with_recipe("metis_det_dark")
             .build())

distortion_task = (task("metis_ifu_distortion")
                   .with_main_input(raw_distortion)
                   .with_associated_input(calib_pinhole)
                   .with_associated_input(dark_task)
                   .with_input_filter(master_dark_class)
                   .with_recipe("metis_ifu_distortion")
                   .build())

wave_task = (task("metis_ifu_wavecal")
             .with_main_input(raw_wave)
             .with_associated_input(calib_persistence)
             .with_associated_input(dark_task)
             .with_input_filter(master_dark_class)
             .with_associated_input(distortion_task)
             .with_input_filter(distortion_table_class)
             .with_recipe("metis_ifu_wavecal")
             .build())

rsrf_task = (task("metis_ifu_rsrf")
             .with_main_input(raw_rsrf)
             .with_associated_input(calib_persistence)
             .with_associated_input(dark_task)
             .with_input_filter(master_dark_class)
             .with_associated_input(distortion_task)
             .with_input_filter(distortion_table_class)
             .with_associated_input(wave_task)
             .with_recipe("metis_ifu_rsrf")
             .build())

std_task = (task("metis_std_reduce")
            .with_main_input(raw_std)
            .with_associated_input(lingain_task)
            .with_input_filter(lin_det_class)
            .with_associated_input(calib_persistence)
            .with_associated_input(dark_task)
            .with_input_filter(master_dark_class)
            .with_associated_input(distortion_task)
            .with_input_filter(distortion_table_class)
            .with_associated_input(wave_task)
            .with_associated_input(rsrf_task)
            .with_input_filter(calib_rsrf_class)
            .with_recipe("metis_ifu_reduce")
            .build())

sci_task = (task("metis_sci_reduce")
            .with_main_input(raw_sci)
            .with_associated_input(lingain_task)
            .with_input_filter(lin_det_class)
            .with_associated_input(calib_persistence)
            .with_associated_input(dark_task)
            .with_input_filter(master_dark_class)
            .with_associated_input(distortion_task)
            .with_input_filter(distortion_table_class)
            .with_associated_input(wave_task)
            .with_associated_input(rsrf_task)
            .with_input_filter(calib_rsrf_class)
            .with_recipe("metis_ifu_reduce")
            .build())

telluric_sci_task = (task("metis_sci_telluric")
                .with_main_input(sci_task)
                .with_input_filter(sci_comb_class)
                .with_associated_input(calib_flux_std)
                .with_associated_input(calib_lsf_kernel)
                .with_associated_input(calib_atm_profile)
                .with_recipe("metis_ifu_telluric")
                .build())

telluric_std_task = (task("metis_std_telluric")
                 .with_main_input(std_task)
                 .with_input_filter(std_comb_class)
                 .with_associated_input(calib_flux_std)
                 .with_associated_input(calib_lsf_kernel)
                 .with_associated_input(calib_atm_profile)
                 .with_recipe("metis_ifu_telluric")
                 .build())

calibrate_task = (task("metis_ifu_calibrate")
                  .with_main_input(sci_task)
                  .with_associated_input(telluric_sci_task)
                  .with_input_filter(sci_telluric_class)
                  .with_associated_input(telluric_std_task)
                  .with_recipe("metis_ifu_calibrate")
                  .build())

post_process_task = (task("metis_ifu_postprocess")
                     .with_main_input(calibrate_task)
                     .with_recipe("metis_ifu_postprocess")
                     .build())