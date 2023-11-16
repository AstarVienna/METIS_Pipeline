from edps import data_source
from edps.generator.time_range import *

from .aspresso_classification import *

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
setup = [kwd.det_binx, kwd.det_biny, kwd.ins_mode]
instrument_setup = [kwd.instrume] + setup

# --- Raw files data sources -----------------------------------------------------------------------

# Raw files for detector linearity with lamp on:
raw_detlin_on = (data_source()
                 .with_classification_rule(on_raw_class)
                 .with_setup_keywords(setup)
                 .with_min_group_size(15)
                 .with_grouping_keywords(instrument_setup + [kwd.tpl_start]).build())

raw_detlin_off = (data_source()
                  .with_classification_rule(off_raw_class)
                  .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.03, 0.03), level=0)
                  .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3)
                  .with_grouping_keywords(instrument_setup + [kwd.tpl_start]).build())

# bias
raw_bias = (data_source()
            .with_classification_rule(bias_class)
            .with_grouping_keywords([kwd.tpl_start])
            .with_min_group_size(5)
            .with_setup_keywords(setup)
            .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1.0), level=0)
            .with_match_keywords(instrument_setup, time_range=TWO_WEEKS, level=1)
            .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
            .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())
# darks
raw_dark = (data_source()
            .with_classification_rule(dark_class)
            .with_grouping_keywords([kwd.tpl_start])
            .with_min_group_size(5)
            .with_setup_keywords(setup)
            .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=0)
            .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=1).build())

# flat field from led source, used for tracing the fibres on the detector
raw_led_ff = (data_source()
              .with_classification_rule(led_ff_class)
              .with_grouping_keywords([kwd.tpl_start])
              .with_min_group_size(15)
              .with_setup_keywords(setup)
              .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-60, 60), level=0)
              .with_match_keywords(instrument_setup, time_range=QUARTERLY, level=1)
              .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# order definition
raw_orderdef = (data_source("ORDERDEF")
                .with_classification_rule(orderdef_a_class)
                .with_classification_rule(orderdef_b_class)
                .with_grouping_keywords([kwd.tpl_start])
                .with_min_group_size(2)
                .with_setup_keywords(setup)
                .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1.0), level=0)
                .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
                .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# flats
raw_flat = (data_source("FLAT")
            .with_classification_rule(flat_a_class)
            .with_classification_rule(flat_b_class)
            .with_grouping_keywords([kwd.tpl_start])
            .with_min_group_size(20)
            .with_setup_keywords(setup)
            .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1.0), level=-0)
            .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
            .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
            .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# - raw files to measure mutual contamination between fibres -
raw_contam_thar = (data_source()
                   .with_classification_rule(contam_thar_class)
                   .with_grouping_keywords([kwd.unique])
                   .with_setup_keywords(setup)
                   .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1.0), level=0)
                   .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                   .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
                   .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

raw_contam_fp = (data_source()
                 .with_classification_rule(contam_fp_class)
                 .with_grouping_keywords([kwd.unique])
                 .with_setup_keywords(setup)
                 .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1.0), level=0)
                 .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                 .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
                 .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())
# --

# - Raw files for wavelength calibration -
raw_wave_fp_fp = (data_source()
                  .with_classification_rule(fp_fp_class)
                  .with_grouping_keywords([kwd.unique])
                  .with_setup_keywords(setup)
                  .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1), level=0)
                  .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                  .with_match_keywords(instrument_setup, time_range=TWO_WEEKS, level=2)
                  .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

raw_wave_thar_fp = (data_source()
                    .with_classification_rule(thar_fp_class)
                    .with_grouping_keywords([kwd.unique])
                    .with_setup_keywords(setup)
                    .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1), level=0)
                    .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                    .with_match_keywords(instrument_setup, time_range=ONE_MONTH, level=2)
                    .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

raw_wave_fp_thar = (data_source()
                    .with_classification_rule(fp_thar_class)
                    .with_grouping_keywords([kwd.unique])
                    .with_setup_keywords(setup)
                    .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1), level=0)
                    .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                    .with_match_keywords(instrument_setup, time_range=TWO_WEEKS, level=2)
                    .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

raw_wave_lfc_fp = (data_source()
                   .with_classification_rule(lfc_fp_class)
                   .with_grouping_keywords([kwd.unique])
                   .with_setup_keywords(setup)
                   .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1), level=0)
                   .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                   .with_match_keywords(instrument_setup, time_range=TWO_WEEKS, level=1)
                   .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

raw_wave_fp_lfc = (data_source()
                   .with_classification_rule(fp_lfc_class)
                   .with_grouping_keywords([kwd.unique])
                   .with_setup_keywords(setup)
                   .with_match_keywords(instrument_setup, time_range=RelativeTimeRange(-0.3, 1), level=0)
                   .with_match_keywords(instrument_setup, time_range=ONE_DAY, level=1)
                   .with_match_keywords(instrument_setup, time_range=TWO_WEEKS, level=2)
                   .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())
# ---

# Raw file to compute fibre's relative efficiencies
raw_eff_ab = (data_source()
              .with_classification_rule(eff_ab_class)
              .with_grouping_keywords([kwd.unique])
              .with_setup_keywords(setup + [kwd.ocs_enabled_fe])
              .with_match_function(rules.assoc_setup_and_telescope, time_range=SAME_NIGHT, level=-1)
              .with_match_function(rules.assoc_setup_and_telescope, time_range=QUARTERLY, level=0)
              .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# standard star
raw_std_flux = (data_source()
                .with_classification_rule(std_flux_class)
                .with_grouping_keywords([kwd.unique])
                .with_setup_keywords(setup + [kwd.telescop])
                .with_match_function(rules.assoc_setup_and_telescope, time_range=SAME_NIGHT, level=-1)
                .with_match_function(rules.assoc_setup_and_telescope, time_range=ONE_MONTH, level=0)
                .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# science exposure
raw_science = (data_source("OBJECT")
               .with_classification_rule(science_fp_class)
               .with_classification_rule(science_thar_class)
               .with_classification_rule(science_sky_class)
               .with_setup_keywords(setup)
               .with_grouping_keywords([kwd.unique]).build())

radial_velocity_star = (data_source("RADIAL_VELOCITY")
                        .with_classification_rule(radial_velocity_class)
                        .with_setup_keywords(setup)
                        .with_grouping_keywords([kwd.unique]).build())

# --- data sources for static calibrations ---------------------------------------------------------

ref_line_table_b = (data_source()
                    .with_classification_rule(ref_line_table_b_class)
                    .with_setup_keywords(setup)
                    .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                    .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

ccd_geom = (data_source()
            .with_classification_rule(ccd_geom_class)
            .with_setup_keywords(setup)
            .with_match_keywords([kwd.det_binx, kwd.det_biny], time_range=IN_THE_PAST)
            .with_match_keywords([kwd.det_binx, kwd.det_biny], time_range=UNLIMITED, level=3).build())

ref_line_table_a = (data_source()
                    .with_setup_keywords(setup)
                    .with_classification_rule(ref_line_table_a_class)
                    .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                    .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

static_dll_matrix_a = (data_source()
                       .with_classification_rule(static_dll_matrix_a_class)
                       .with_setup_keywords(setup)
                       .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                       .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

std_table = (data_source()
             .with_classification_rule(std_table_class)
             .with_match_keywords([kwd.instrume], time_range=IN_THE_PAST, level=0)
             .with_match_keywords([kwd.instrume], time_range=UNLIMITED, level=3).build())

mask_lut = (data_source()
            .with_classification_rule(mask_lut_class)
            .with_match_keywords([kwd.instrume], time_range=IN_THE_PAST, level=0)
            .with_match_keywords([kwd.instrume], time_range=UNLIMITED, level=3).build())

ext_table = (data_source()
             .with_classification_rule(ext_table_class)
             .with_match_keywords([kwd.instrume], time_range=IN_THE_PAST, level=0)
             .with_match_keywords([kwd.instrume], time_range=UNLIMITED, level=3).build())

flux_template = (data_source()
                 .with_classification_rule(flux_template_class)
                 .with_setup_keywords(setup)
                 .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                 .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

static_wave_matrix_a = (data_source()
                        .with_classification_rule(static_wave_matrix_a_class)
                        .with_setup_keywords(setup)
                        .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                        .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

inst_config = (data_source()
               .with_classification_rule(inst_config_class)
               .with_setup_keywords(setup)
               .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
               .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

static_wave_matrix_b = (data_source()
                        .with_classification_rule(static_wave_matrix_b_class)
                        .with_setup_keywords(setup)
                        .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                        .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

led_ff_gain_windows = (data_source()
                       .with_classification_rule(led_ff_gain_windows_class)
                       .with_setup_keywords(setup)
                       .with_match_keywords([kwd.det_binx, kwd.det_biny], time_range=IN_THE_PAST, level=0)
                       .with_match_keywords([kwd.det_binx, kwd.det_biny], time_range=UNLIMITED, level=3).build())

wave_line_table_a = (data_source()
                     .with_classification_rule(wave_line_table_a_class)
                     .with_setup_keywords(setup)
                     .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                     .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

static_dll_matrix_b = (data_source()
                       .with_classification_rule(static_dll_matrix_b_class)
                       .with_setup_keywords(setup)
                       .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                       .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

mask_table = (data_source()
              .with_classification_rule(mask_table_class)
              .with_match_function(rules.is_associated, time_range=IN_THE_PAST, level=0)
              .with_grouping_keywords([kwd.instrume]).build())

wave_line_table_b = (data_source()
                     .with_classification_rule(wave_line_table_b_class)
                     .with_setup_keywords(setup)
                     .with_match_keywords(instrument_setup, time_range=IN_THE_PAST, level=0)
                     .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())

# --- Other data sources ---------------------------------------------------------------------------

# The cosmic ray mask is a dynamic bad pixel mask that is generated by the task 'object'. However,
# it can also provided as an input data source:
cosmic_rays_mask = (data_source()
                    .with_classification_rule(cosmic_rays_mask_class)
                    .with_setup_keywords(setup)
                    .with_match_keywords([kwd.mjd_obs], level=0)
                    .with_match_keywords(instrument_setup, time_range=UNLIMITED, level=3).build())
