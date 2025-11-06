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


# --- LM IMG Data sources ---
detlin_2rg_raw = (data_source()
            .with_classification_rule(detlin_2rg_raw_class)
            .with_match_keywords(["instrume"])
            .build())

dark_2rg_raw = (data_source()
            .with_classification_rule(dark_2rg_raw_class)
            .with_match_keywords(["instrume"])
            .build())

lm_flat_lamp_raw = (data_source()
            .with_classification_rule(lm_flat_lamp_raw_class)
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

lm_image_sci_raw = (data_source()
            .with_classification_rule(lm_image_sci_raw_class)
            .with_match_keywords(["instrume"])
            .build())

lm_image_sky_raw = (data_source()
            .with_classification_rule(lm_image_sky_raw_class)
            .with_match_keywords(["instrume"])
            .build())

lm_image_std_raw = (data_source()
            .with_classification_rule(lm_image_std_raw_class)
            .with_match_keywords(["instrume"])
            .build())

# ------- N IMG BAND DATA SOURCES ---------

detlin_geo_raw = (data_source()
            .with_classification_rule(detlin_geo_raw_class)
            .with_match_keywords(["instrume"])
            .build())

dark_geo_raw = (data_source()
            .with_classification_rule(dark_geo_raw_class)
            .with_grouping_keywords(["instrume","tpl.start"])
            .with_match_keywords(["instrume","det.dit","det.ndit"])
            .build())

n_flat_lamp_raw = (data_source()
            .with_classification_rule(n_flat_lamp_raw_class)
            .with_grouping_keywords(["instrume","tpl.start"])
            .with_match_keywords(["instrume"])
            .build())

n_distortion_raw = (data_source()
            .with_classification_rule(n_distortion_raw_class)
            .with_match_keywords(["instrume"])
            .build())

n_wcu_off_raw = (data_source()
            .with_classification_rule(n_wcu_off_raw_class)
            .with_match_keywords(["instrume"])
            .build())

n_image_sci_raw = (data_source()
            .with_classification_rule(n_image_sci_raw_class)        
            .with_match_keywords(["instrume"])
            .build())

n_image_sky_raw = (data_source()
            .with_classification_rule(n_image_sky_raw_class)        
            .with_match_keywords(["instrume"])
            .build())

n_image_std_raw = (data_source()
            .with_classification_rule(n_image_std_raw_class)        
            .with_match_keywords(["instrume"])
            .build())


fluxstd_catalog = (data_source()
                .with_classification_rule(fluxstd_catalog_class)
                .with_match_keywords(["instrume"])
                .build())

# --- IFU Data Sources ---

badpix_map_ifu = (data_source()
                 .with_classification_rule(badpix_map_ifu_class)
                 .build())

detlin_ifu_raw = (data_source()
              .with_classification_rule(detlin_ifu_raw_class)
              .with_match_keywords(["instrume"])
              .build())

dark_ifu_raw = (data_source()
            .with_classification_rule(dark_ifu_raw_class)
            .with_match_keywords(["instrume"])
            .build())

ifu_distortion_raw = (data_source()
                  .with_classification_rule(ifu_distortion_raw_class)
                  .with_match_keywords(["instrume"])
                  .build())

ifu_wave_raw = (data_source()
            .with_classification_rule(ifu_wave_raw_class)
            .with_match_keywords(["instrume"])
            .build())

ifu_rsrf_raw = (data_source()
            .with_classification_rule(ifu_rsrf_raw_class)
            .with_match_keywords(["instrume"])
            .build())

ifu_wcu_off_raw = (data_source()
                   .with_classification_rule(ifu_wcu_off_raw_class)
                   .with_match_keywords(["instrume"])
                   .build())

ifu_std_raw = (data_source()
           .with_classification_rule(ifu_std_raw_class)
           .with_match_keywords(["instrume"])
           .build())

ifu_sky_raw = (data_source()
               .with_classification_rule(ifu_sky_raw_class)
               .with_match_keywords(["instrume"])
               .build())

ifu_sci_raw = (data_source()
           .with_classification_rule(ifu_sci_raw_class)
           .with_match_keywords(["instrume"])
           .build())

persistence_map = (data_source()
                     .with_classification_rule(persistence_map_class)
                     .with_match_keywords(["simple"])
                     .build())

pinhole_table = (data_source()
                 .with_classification_rule(pinhole_table_class)
                 .with_match_keywords(["simple"])
                 .build())

lsf_kernel = (data_source()
                    .with_classification_rule(lsf_kernel_class)
                    .with_match_keywords(["simple"])
                    .build())

fluxstd_catalog = (data_source()
                  .with_classification_rule(fluxstd_catalog_class)
                  .with_match_keywords(["simple"])
                  .build())

atm_profile = (data_source()
                     .with_classification_rule(atm_profile_class)
                     .with_match_keywords(["simple"])
                     .build())

ifu_wavecal = (data_source()
            .with_classification_rule(ifu_wavecal_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

# --- LM LSS Data Sources ---
lm_adc_slitloss_raw = (data_source()
            .with_classification_rule(lm_adc_slitloss_raw_class)
            # .with_min_group_size(MIN_NUM_SLITLOSPOS)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_rsrf_raw = (data_source()
            .with_classification_rule(lm_lss_rsrf_raw_class)
            #.with_min_group_size(MIN_NUM_RSRF)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_rsrf_pinh_raw = (data_source()
            .with_classification_rule(lm_lss_rsrf_pinh_raw_class)
            #.with_min_group_size(MIN_NUM_PINH)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_std_raw = (data_source()
            .with_classification_rule(lm_lss_std_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_sci_raw = (data_source()
            .with_classification_rule(lm_lss_sci_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_wave_raw = (data_source()
            .with_classification_rule(lm_lss_wave_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_sci_flux_1d = (data_source()
            .with_classification_rule(lm_lss_sci_flux_1d_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

# STATIC + OTHER calibration sources (LM/LSS)
gain_map_h2rg = (data_source()
            .with_classification_rule(gain_map_h2rg_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

linearity_h2rg = (data_source()
            .with_classification_rule(linearity_h2rg_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

badpix_map_h2rg = (data_source()
            .with_classification_rule(badpix_map_h2rg_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

atm_line_cat = (data_source()
            .with_classification_rule(atm_line_cat_class)
            .with_match_keywords([metis_kwd.instrume])
            # .with_match_keywords([metis_kwd.pro_catg])
            .build())

laser_tab = (data_source()
            .with_classification_rule(laser_tab_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

ref_std_cat = (data_source()
            .with_classification_rule(ref_std_cat_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_dist_sol = (data_source()
            .with_classification_rule(lm_lss_dist_sol_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_wave_guess = (data_source()
            .with_classification_rule(lm_lss_wave_guess_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_lss_synth_trans = (data_source()
            .with_classification_rule(lm_lss_synth_trans_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

ao_psf_model = (data_source()
            .with_classification_rule(ao_psf_model_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

lm_adc_slitloss = (data_source()
            .with_classification_rule(lm_adc_slitloss_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

mf_best_fit_tab = (data_source()
            .with_classification_rule(mf_best_fit_tab_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

# --- N LSS Data Sources ---

n_adc_slitloss_raw = (data_source()
            .with_classification_rule(n_adc_slitloss_raw_class)
            # .with_min_group_size(MIN_NUM_SLITLOSPOS)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_rsrf_raw = (data_source()
            .with_classification_rule(n_lss_rsrf_raw_class)
            #.with_min_group_size(MIN_NUM_RSRF)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_rsrf_pinh_raw = (data_source()
            .with_classification_rule(n_lss_rsrf_pinh_raw_class)
            #.with_min_group_size(MIN_NUM_PINH)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_std_raw = (data_source()
            .with_classification_rule(n_lss_std_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_wave_raw = (data_source()
            .with_classification_rule(n_lss_wave_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_sci_raw = (data_source()
            .with_classification_rule(n_lss_sci_raw_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

# Final data products
n_lss_sci_flux_1d = (data_source()
            .with_classification_rule(n_lss_sci_flux_1d_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

gain_map_geo = (data_source()
            .with_classification_rule(gain_map_geo_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

linearity_geo = (data_source()
            .with_classification_rule(linearity_geo_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

badpix_map_geo = (data_source()
            .with_classification_rule(badpix_map_geo_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

ref_std_cat = (data_source()
            .with_classification_rule(ref_std_cat_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_dist_sol = (data_source()
            .with_classification_rule(n_lss_dist_sol_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_wave_guess = (data_source()
            .with_classification_rule(n_lss_wave_guess_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_lss_synth_trans = (data_source()
            .with_classification_rule(n_lss_synth_trans_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

n_adc_slitloss = (data_source()
            .with_classification_rule(n_adc_slitloss_class)
            .with_match_keywords([metis_kwd.instrume])
            .build())

