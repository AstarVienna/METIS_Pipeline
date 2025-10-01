from edps import data_source
from edps.generator.time_range import *

from .metis_classification import *

# Convention for Data sources Association rule levels:
# Each data source can have several match function which correspond to different
# quality levels for the selected data. The level is specified as a number that
# follows this convention:
#   level < 0: more restrictive than the calibration plan
#   level = 0 follows the calibration plan
#   level = 1 quality sufficient for QC1 certification
#   level = 2 probably still acceptable quality
#   level = 3 significant risk of bad quality results

# standard matching keywords:
setup = [metis_kwd.det_binx, metis_kwd.det_biny, metis_kwd.ins_mode]
instrument_setup = [metis_kwd.instrume] + setup


# --- Data sources ---
detlin_2rg_raw = (data_source()
            .with_classification_rule(detlin_2rg_raw_class)
            .with_match_keywords(["instrume"])
            .build())

raw_2rg_dark = (data_source()
            .with_classification_rule(rawdark_2rg_class)
            .with_match_keywords(["instrume"])
            .build())

lm_lamp_flat = (data_source()
            .with_classification_rule(lm_lampflat_class)
            .with_match_keywords(["instrume"])
            .build())

lm_distortion_raw = (data_source()
            .with_classification_rule(lm_distortion_raw_class)
            .with_match_keywords(["instrume"])
            .build())

lm_wcu_off_raw = (data_source()
            .with_classification_rule(lm_wcu_off_raw_class)
            .with_match_keywords(["instrume"])
            .build())

lm_raw_science = (data_source()
            .with_classification_rule(lm_raw_science_class)
            .with_match_keywords(["instrume"])
            .build())

lm_raw_sky = (data_source()
            .with_classification_rule(lm_raw_sky_class)
            .with_match_keywords(["instrume"])
            .build())

lm_raw_std = (data_source()
            .with_classification_rule(lm_raw_std_class)
            .with_match_keywords(["instrume"])
            .build())
# ------- N BAND DATA SOURCES ---------

detlin_geo_raw = (data_source()
            .with_classification_rule(detlin_geo_raw_class)
            .with_match_keywords(["instrume"])
            .build())

raw_geo_dark = (data_source()
            .with_classification_rule(rawdark_geo_class)
            .with_grouping_keywords(["instrume","tpl.start"])
            .with_match_keywords(["instrume","det.dit","det.ndit"])
            .build())

n_lamp_flat = (data_source()
            .with_classification_rule(n_lampflat_class)
            .with_grouping_keywords(["instrume","tpl.start"])
            .with_match_keywords(["instrume","det.dit","det.ndit"])
            .build())

n_distortion_raw = (data_source()
            .with_classification_rule(n_distortion_raw_class)
            .with_match_keywords(["instrume"])
            .build())

n_wcu_off_raw = (data_source()
            .with_classification_rule(n_wcu_off_raw_class)
            .with_match_keywords(["instrume"])
            .build())

n_raw_science = (data_source()
            .with_classification_rule(n_raw_science_class)        
            .with_match_keywords(["instrume"])
            .build())

n_raw_sky = (data_source()
            .with_classification_rule(n_raw_sky_class)        
            .with_match_keywords(["instrume"])
            .build())

n_raw_std = (data_source()
            .with_classification_rule(n_raw_std_class)        
            .with_match_keywords(["instrume"])
            .build())


fluxstd_catalog = (data_source()
                .with_classification_rule(fluxstd_catalog_class)
                .with_match_keywords(["instrume"])
                .build())

pinehole_tab = (data_source()
                .with_classification_rule(pinhole_table_class)
                .with_match_keywords(["instrume"])
                .build())

# --- IFU Data Sources ---

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
                 .with_classification_rule(pinhole_table_class)
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
