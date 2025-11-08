# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: V0.1 (Skeleton level)
#
#   TODO: Next steps: Implement telluric correction branches
#
#

"""METIS LSS LM-Band workflow"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import subworkflow, task, data_source, classification_rule
from .metis_classification import *
from .metis_datasources import *
from .metis_task_functions import *
from .metis_lm_lss_molecfit import telluric_on_standard, telluric_on_science

# ----------------------------------------------------------------------------
# ------------------------ Processing tasks ----------------------------------
# ----------------------------------------------------------------------------

# persistence_task = (task('metis_det_persistence')
#             .with_main_input(raw_slitloss)
#             .with_recipe("metis_det_persistence")
#             .build())

lm_lss_lingain_task = (task('metis_lm_lss_lingain')
            .with_recipe("metis_det_lingain")
            .with_main_input(detlin_2rg_raw)
            .with_associated_input(lm_wcu_off_raw)        # check how that is exactly delivered by the ICS!
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_dark_task = (task('metis_lm_lss_dark')
            .with_recipe("metis_det_dark")
            .with_main_input(dark_2rg_raw)       # check what the minimum number of raw darks is!
            .with_associated_input(lm_lss_lingain_task)
            .with_associated_input(persistence_map)
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_adc_slitloss_task = (task('metis_lm_lss_adc_slitloss')
            .with_recipe("metis_lm_adc_slitloss")
            .with_main_input(lm_adc_slitloss_raw)
            .with_associated_input(lm_wcu_off_raw)        # check how that is exactly delivered by the ICS!
            .with_associated_input(persistence_map)
            .with_associated_input(lm_lss_lingain_task)
            .with_associated_input(lm_lss_dark_task)
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_rsrf_task = (task('metis_lm_lss_rsrf')
            .with_recipe("metis_lm_lss_rsrf")
            .with_main_input(lm_lss_rsrf_raw)
            .with_associated_input(lm_wcu_off_raw)
            .with_associated_input(lm_lss_dark_task)
            .with_associated_input(lm_lss_lingain_task)
            .with_associated_input(persistence_map)
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_trace_task = (task("metis_lm_lss_trace")
            .with_recipe("metis_lm_lss_trace")
            .with_main_input(lm_lss_rsrf_pinh_raw)
            .with_associated_input(lm_wcu_off_raw)
            .with_associated_input(lm_lss_dark_task)
            .with_associated_input(lm_lss_lingain_task)
            .with_associated_input(lm_lss_rsrf_task)
            .with_associated_input(persistence_map, min_ret=0)
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_wave_task = (task("metis_lm_lss_wave")
            .with_recipe("metis_lm_lss_wave")
            .with_main_input(lm_lss_wave_raw)
            .with_associated_input(lm_wcu_off_raw)
            .with_associated_input(persistence_map, min_ret=0)
            .with_associated_input(laser_tab)
            .with_associated_input(lm_lss_dark_task)
            .with_associated_input(lm_lss_lingain_task)
            .with_associated_input(lm_lss_rsrf_task)
            .with_associated_input(lm_lss_trace_task)
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_std_task = (task('metis_lm_lss_std')
            .with_main_input(lm_lss_std_raw)
            # .with_associated_input(linearity_task)
            .with_associated_input(badpix_map_h2rg)
            .with_associated_input(gain_map_h2rg)
            .with_associated_input(linearity_h2rg)
            .with_associated_input(persistence_map, min_ret=0)
            .with_associated_input(lm_lss_dark_task)
            .with_associated_input(lm_lss_rsrf_task)
            .with_associated_input(lm_lss_trace_task)
            .with_associated_input(lm_lss_wave_task)
            .with_associated_input(lm_lss_adc_slitloss_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(ref_std_cat)
            .with_associated_input(lm_lss_synth_trans)
            .with_associated_input(lm_lss_dist_sol)
            .with_associated_input(lm_lss_wave_guess)
            .with_associated_input(lm_adc_slitloss)
            .with_associated_input(ao_psf_model)
            .with_recipe('metis_lm_lss_std')
            .with_meta_targets([SCIENCE])
            .build())

lm_lss_sci_task = (task('metis_lm_lss_sci')
            .with_main_input(lm_lss_sci_raw)
            # .with_associated_input(linearity_task)
            .with_associated_input(badpix_map_h2rg)
            .with_associated_input(gain_map_h2rg)
            .with_associated_input(linearity_h2rg)
            .with_associated_input(persistence_map, min_ret=0)
            .with_associated_input(lm_lss_dark_task)
            .with_associated_input(lm_lss_rsrf_task)
            .with_associated_input(lm_lss_trace_task)
            .with_associated_input(lm_lss_wave_task)
            .with_associated_input(lm_lss_dist_sol)
            .with_associated_input(lm_lss_wave_guess)
            .with_associated_input(lm_lss_std_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(lm_adc_slitloss)
            .with_associated_input(ao_psf_model)
            .with_recipe('metis_lm_lss_sci')
            .with_meta_targets([SCIENCE])
            .build())

# TODO: Implement the different telluric corr branches
# If the parameter "molecfit"="standard", then the atmospheric transmission is computed on the spectrum
# of the standard star. This is done in the subworkflow telluric_on_standard.
# transmission_from_standard = telluric_on_standard(std_reduction_task, sci_reduction_task)

# If the parameter "molecfit"="science", then the atmospheric transmission is directly computed on
# the science spectrum. This is done in the subworkflow telluric_on_science.
# transmission_from_science = telluric_on_science(sci_reduction_task)

# Include branch to let user decide whether science of std data as input --> ask Lodo for branching!
lm_lss_mf_model_task = (task("metis_lm_lss_mf_model")
            .with_main_input(lm_lss_sci_task) 
            # .with_associated_input(static_ao_psf_model)
            .with_associated_input(atm_line_cat)
            .with_associated_input(lsf_kernel)
            .with_associated_input(atm_profile)
            # TODO: Include output filtering for sci & std outputs
            .with_associated_input(lm_lss_sci_task, condition=on_science)
            .with_associated_input(lm_lss_std_task, condition=on_standard)
            .with_recipe('metis_lm_lss_mf_model')
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_mf_calctrans_task = (task("metis_lm_lss_mf_calctrans")
            .with_main_input(lm_lss_mf_model_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(lsf_kernel)
            .with_associated_input(atm_profile)
            .with_recipe('metis_lm_lss_mf_calctrans')
            .with_meta_targets([QC1_CALIB])
            .build())

lm_lss_mf_correct_task = (task("metis_lm_lss_mf_correct")
            .with_main_input(lm_lss_sci_task)
            .with_associated_input(lm_lss_mf_calctrans_task)
            .with_recipe("metis_lm_lss_mf_correct")
            .with_meta_targets([QC1_CALIB, SCIENCE])
            .build())




