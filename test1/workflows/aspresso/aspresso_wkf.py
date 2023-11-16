from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, ReportInput

from .aspresso_datasources import *
from .aspresso_detector_properties import detector_properties
from .aspresso_flats_and_tracing import flats_and_tracing
from .aspresso_science import science_reduction

bias, dark, detmon = detector_properties()
led_ff, orderdef, flat = flats_and_tracing(dark)

# --- Tasks to compute the mutual contamination and relative efficiencies between fibers -------------------------------
contam_thar = (task('contam_thar')
               .with_recipe('espdr_cal_contam')
               .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
               .with_report('aspresso_contamination', ReportInput.RECIPE_INPUTS_OUTPUTS)
               .with_main_input(raw_contam_thar)
               .with_associated_input(bias, [MASTER_BIAS_RES])
               .with_associated_input(dark, [HOT_PIXEL_MASK])
               .with_associated_input(led_ff, [BAD_PIXEL_MASK])
               .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
               .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B])
               .with_associated_input(ccd_geom)
               .with_associated_input(inst_config)
               .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                  ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, inst_config_class,
                                  ccd_geom_class)
               .with_meta_targets([QC1_CALIB])
               .build())

contam_fp = (task('contam_fp')
             .with_recipe('espdr_cal_contam')
             .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
             .with_report('aspresso_contamination', ReportInput.RECIPE_INPUTS_OUTPUTS)
             .with_main_input(raw_contam_fp)
             .with_associated_input(bias, [MASTER_BIAS_RES])
             .with_associated_input(dark, [HOT_PIXEL_MASK])
             .with_associated_input(led_ff, [BAD_PIXEL_MASK])
             .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
             .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B])
             .with_associated_input(ccd_geom)
             .with_associated_input(inst_config)
             .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, inst_config_class,
                                ccd_geom_class)
             .with_meta_targets([QC1_CALIB])
             .build())

eff_ab = (task('eff_ab')
          .with_recipe('espdr_cal_eff_ab')
          .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
          .with_report('aspresso_skyflat_eff', ReportInput.RECIPE_INPUTS_OUTPUTS)
          .with_main_input(raw_eff_ab)
          .with_associated_input(dark, [HOT_PIXEL_MASK])
          .with_associated_input(led_ff, [BAD_PIXEL_MASK])
          .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
          .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B])
          .with_associated_input(ccd_geom)
          .with_associated_input(inst_config)
          .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                             ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, inst_config_class,
                             ccd_geom_class)
          .with_meta_targets([QC1_CALIB, CALCHECKER])
          .build())
# ----------------------------------------------------------------------------------------------------------------------

# --- Tasks for wavelength calibration ---------------------------------------------------------------------------------
wave_fp_fp = (task('wave_fp_fp')
              .with_recipe('espdr_wave_FP')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
              .with_report('aspresso_wavelength_FP', ReportInput.RECIPE_INPUTS_OUTPUTS)
              .with_main_input(raw_wave_fp_fp)
              .with_associated_input(dark, [HOT_PIXEL_MASK])
              .with_associated_input(led_ff, [BAD_PIXEL_MASK])
              .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
              .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B,
                                            BLAZE_A, BLAZE_B])
              .with_associated_input(ccd_geom)
              .with_associated_input(inst_config)
              .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                 ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                 inst_config_class)
              .with_meta_targets([QC1_CALIB])
              .build())

wave_thar_fp = (task('wave_thar_fp')
                .with_recipe('espdr_wave_THAR')
                .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
                .with_report('aspresso_wavelength_THAR', ReportInput.RECIPE_INPUTS_OUTPUTS)
                .with_main_input(raw_wave_thar_fp)
                .with_associated_input(dark, [HOT_PIXEL_MASK])
                .with_associated_input(led_ff, [BAD_PIXEL_MASK])
                .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
                .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B,
                                              BLAZE_A, BLAZE_B])
                .with_associated_input(wave_fp_fp, [FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B])
                .with_associated_input(ccd_geom)
                .with_associated_input(inst_config)
                .with_associated_input(ref_line_table_a)
                .with_associated_input(ref_line_table_b)
                .with_associated_input(static_wave_matrix_b)
                .with_associated_input(static_dll_matrix_b)
                .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                   ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                   FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B,
                                   S2D_BLAZE_FP_FP_A, S2D_BLAZE_FP_FP_B, inst_config_class,
                                   static_dll_matrix_a_class, static_dll_matrix_b_class,
                                   static_wave_matrix_a_class, static_wave_matrix_b_class)
                .with_meta_targets([QC1_CALIB])
                .build())

wave_fp_thar = (task('wave_fp_thar')
                .with_recipe('espdr_wave_THAR')
                .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
                .with_report('aspresso_wavelength_THAR', ReportInput.RECIPE_INPUTS_OUTPUTS)
                .with_main_input(raw_wave_fp_thar)
                .with_associated_input(dark, [HOT_PIXEL_MASK])
                .with_associated_input(led_ff, [BAD_PIXEL_MASK])
                .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
                .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B,
                                              BLAZE_A, BLAZE_B])
                .with_associated_input(wave_fp_fp, [FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B,
                                                    S2D_BLAZE_FP_FP_A, S2D_BLAZE_FP_FP_B])
                .with_associated_input(ccd_geom)
                .with_associated_input(inst_config)
                .with_associated_input(ref_line_table_a)
                .with_associated_input(ref_line_table_b)
                .with_associated_input(static_wave_matrix_a)
                .with_associated_input(static_dll_matrix_a)
                .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                   ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                   FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B,
                                   S2D_BLAZE_FP_FP_A, S2D_BLAZE_FP_FP_B, inst_config_class,
                                   static_dll_matrix_a_class, static_dll_matrix_b_class,
                                   static_wave_matrix_a_class, static_wave_matrix_b_class)
                .with_meta_targets([QC1_CALIB])
                .build())

wave_lfc_fp = (task('wave_lfc_fp')
               .with_recipe('espdr_wave_LFC')
               .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
               .with_report('aspresso_wavelength_LFC', ReportInput.RECIPE_INPUTS_OUTPUTS)
               .with_main_input(raw_wave_lfc_fp)
               .with_associated_input(dark, [HOT_PIXEL_MASK])
               .with_associated_input(led_ff, [BAD_PIXEL_MASK])
               .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
               .with_associated_input(flat,
                                      [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B])
               .with_associated_input(wave_thar_fp, [WAVE_MATRIX_THAR_FP_A, DLL_MATRIX_THAR_FP_A])
               .with_associated_input(ccd_geom)
               .with_associated_input(inst_config)
               .with_input_filter(HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                  ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                  FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B,
                                  S2D_BLAZE_FP_FP_A, S2D_BLAZE_FP_FP_B, WAVE_MATRIX_THAR_FP_A, DLL_MATRIX_THAR_FP_A)
               .with_meta_targets([QC1_CALIB])
               .build())

wave_fp_lfc = (task('wave_fp_lfc')
               .with_recipe('espdr_wave_LFC')
               .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
               .with_report('aspresso_wavelength_LFC', ReportInput.RECIPE_INPUTS_OUTPUTS)
               .with_main_input(raw_wave_fp_lfc)
               .with_associated_input(dark, [HOT_PIXEL_MASK])
               .with_associated_input(led_ff, [BAD_PIXEL_MASK])
               .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
               .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B,
                                             BLAZE_A, BLAZE_B])
               .with_associated_input(wave_fp_thar, [WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_FP_THAR_B])
               .with_associated_input(ccd_geom)
               .with_associated_input(inst_config)
               .with_input_filter(HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                  ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                  FP_SEARCHED_LINE_TABLE_FP_FP_A, FP_SEARCHED_LINE_TABLE_FP_FP_B,
                                  S2D_BLAZE_FP_FP_A, S2D_BLAZE_FP_FP_B, WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_FP_THAR_B)
               .with_meta_targets([QC1_CALIB])
               .build())
# ----------------------------------------------------------------------------------------------------------------------

# --- Flux calibration
flux = (task('flux')
        .with_recipe('espdr_cal_flux')
        .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
        .with_report('aspresso_specphot_std', ReportInput.RECIPE_INPUTS_OUTPUTS)
        .with_main_input(raw_std_flux)
        .with_associated_input(dark, [HOT_PIXEL_MASK])
        .with_associated_input(led_ff, [BAD_PIXEL_MASK])
        .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
        .with_associated_input(flat, [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B,
                                      BLAZE_A, BLAZE_B])
        .with_associated_input(wave_thar_fp, [WAVE_MATRIX_THAR_FP_A, DLL_MATRIX_THAR_FP_A])
        .with_associated_input(wave_fp_thar, [WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_FP_THAR_B])
        .with_associated_input(ccd_geom)
        .with_associated_input(inst_config)
        .with_associated_input(std_table)
        .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                           ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                           WAVE_MATRIX_THAR_FP_A, WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_THAR_FP_A, DLL_MATRIX_FP_THAR_B,
                           ext_table_class, std_table_class)
        .with_associated_input(ext_table)
        .with_meta_targets([QC1_CALIB, QC0, CALCHECKER])
        .build())

# --- Reduction of science data and radial velocity stars
rv_stars, science = science_reduction(radial_velocity_star, 'rv_stars', [CALCHECKER], raw_science, 'object',
                                      [QC0, SCIENCE, CALCHECKER], bias, dark, led_ff, orderdef, flat, wave_thar_fp,
                                      wave_fp_thar, wave_lfc_fp,
                                      wave_fp_lfc, eff_ab, flux, contam_thar, contam_fp)
