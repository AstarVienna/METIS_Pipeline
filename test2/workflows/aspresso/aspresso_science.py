from edps import task, ReportInput, subworkflow

from .aspresso_datasources import *


def reduction(main_input, task_name, metatarget_list, bias, dark, led_ff, orderdef, flat, wave_thar_fp, wave_fp_thar,
              wave_lfc_fp, wave_fp_lfc, eff_ab, flux, contam_thar, contam_fp):
    result = (task(task_name)
              .with_recipe('espdr_sci_red')
              .with_report('aspresso_rawdisp', ReportInput.RECIPE_INPUTS)
              .with_main_input(main_input)
              .with_associated_input(bias, [MASTER_BIAS_RES])
              .with_associated_input(dark, [HOT_PIXEL_MASK])
              .with_associated_input(led_ff, [BAD_PIXEL_MASK])
              .with_associated_input(orderdef, [ORDER_TABLE_A, ORDER_TABLE_B])
              .with_associated_input(flat,
                                     [ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B])
              .with_associated_input(wave_thar_fp, [WAVE_MATRIX_THAR_FP_A, DLL_MATRIX_THAR_FP_A, S2D_BLAZE_THAR_FP_B])
              .with_associated_input(wave_fp_thar, [WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_FP_THAR_B])
              .with_associated_input(wave_lfc_fp, [WAVE_MATRIX_LFC_FP_A, DLL_MATRIX_LFC_FP_A, S2D_BLAZE_FP_LFC_A],
                                     min_ret=0)
              .with_associated_input(wave_fp_lfc, [WAVE_MATRIX_FP_LFC_B, DLL_MATRIX_FP_LFC_B, S2D_BLAZE_LFC_FP_B],
                                     min_ret=0)
              .with_associated_input(eff_ab, [REL_EFF_B])
              .with_associated_input(flux, [ABS_EFF_A], min_ret=0)
              .with_associated_input(contam_thar, [MASTER_CONTAM_THAR], min_ret=0, max_ret=1)
              .with_associated_input(contam_fp, [MASTER_CONTAM_FP], min_ret=0, max_ret=1)
              .with_associated_input(ccd_geom)
              .with_associated_input(inst_config)
              .with_associated_input(mask_table, min_ret=11, max_ret=100)
              .with_associated_input(mask_lut)
              .with_associated_input(ext_table)
              .with_associated_input(flux_template)
              .with_associated_input(cosmic_rays_mask, min_ret=0, max_ret=1)
              .with_input_filter(MASTER_BIAS_RES, HOT_PIXEL_MASK, BAD_PIXEL_MASK, ORDER_TABLE_A, ORDER_TABLE_B,
                                 ORDER_PROFILE_A, ORDER_PROFILE_B, FSPECTRUM_A, FSPECTRUM_B, BLAZE_A, BLAZE_B,
                                 WAVE_MATRIX_THAR_FP_A, DLL_MATRIX_THAR_FP_A, WAVE_MATRIX_LFC_FP_A,
                                 DLL_MATRIX_LFC_FP_A,
                                 WAVE_MATRIX_FP_THAR_B, DLL_MATRIX_FP_THAR_B, mask_lut_class, S2D_BLAZE_THAR_FP_B,
                                 MASTER_CONTAM_FP, MASTER_CONTAM_THAR, REL_EFF_B, ABS_EFF_A, mask_table_class,
                                 inst_config_class, cosmic_rays_mask_class)
              .with_meta_targets(metatarget_list)
              .build())

    return result


@subworkflow("science_reduction", "")
def science_reduction(input1, task1, metatarget1, input2, task2, metatarget2, bias, dark, led_ff, orderdef, flat,
                      wave_thar_fp, wave_fp_thar, wave_lfc_fp, wave_fp_lfc, eff_ab, flux, contam_thar, contam_fp):
    rv_stars = reduction(input1, task1, metatarget1, bias, dark, led_ff, orderdef, flat, wave_thar_fp, wave_fp_thar,
                         wave_lfc_fp, wave_fp_lfc, eff_ab, flux, contam_thar, contam_fp)

    science = reduction(input2, task2, metatarget2, bias, dark, led_ff, orderdef, flat, wave_thar_fp, wave_fp_thar,
                        wave_lfc_fp, wave_fp_lfc, eff_ab, flux, contam_thar, contam_fp)

    return rv_stars, science
