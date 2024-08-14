# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

"""METIS LSS LM-Band workflow"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule
from .metis_lm_lss_classification import *
from .metis_lm_lss_datasources import *
from .metis_lm_lss_task_functions import *

# ----------------------------------------------------------------------------
# ------------------------ Processing tasks ----------------------------------
# ----------------------------------------------------------------------------

# persistence_task = (task('metis_det_persistence')
#             .with_main_input(raw_slitloss)
#             .with_recipe("metis_det_persistence")
#             .build())

linearity_task = (task('metis_det_lingain')
            .with_main_input(raw_lingain)
            .with_associated_input(static_persistence_map, min_ret=0) # min_ret=0 --> optional input
            .with_associated_input(h2rg_wcu_off)        # check how that is exactly delivered by the ICS!
            .with_recipe("metis_det_lingain")
            .with_meta_targets([QC1_CALIB])
            .build())

slitloss_task = (task('metis_lm_adc_slitloss')
            .with_main_input(raw_slitloss)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_recipe("metis_lm_adc_slitloss")
            .with_meta_targets([QC1_CALIB])
            .build())

dark_task = (task('metis_det_dark')
            .with_main_input(raw_dark)       # check what the minimum number of raw darks is!
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_recipe("metis_det_dark")
            .with_meta_targets([QC1_CALIB])
            .build())

flatfield_task = (task('metis_LM_lss_rsrf')
            .with_main_input(lm_raw_rsrf)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task, [MASTERDARK_class])
            .with_recipe("metis_LM_lss_rsrf")
            .with_meta_targets([QC1_CALIB])
            .build())

trace_finding_task = (task("metis_LM_lss_trace")
            .with_main_input(lm_raw_rsrf_pinh)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_recipe("metis_LM_lss_trace")
            .with_meta_targets([QC1_CALIB])
            .build())

wavelength_calibration_task = (task("metis_LM_lss_wave")
            .with_main_input(lm_raw_wave)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_associated_input(flatfield_task)
            .with_associated_input(trace_finding_task)
            .with_associated_input(static_laser_tab)
            .with_recipe("metis_LM_lss_wave")
            .with_meta_targets([QC1_CALIB])
            .build())

std_reduction_task = (task('metis_LM_lss_std')
            .with_main_input(lm_raw_std)
            # .with_associated_input(linearity_task)
            .with_associated_input(static_badpix_map_h2rg)
            .with_associated_input(static_gain_map_h2rg)
            .with_associated_input(static_linearity_h2rg)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_associated_input(flatfield_task)
            .with_associated_input(trace_finding_task)
            .with_associated_input(wavelength_calibration_task)
            .with_associated_input(slitloss_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_ref_std_cat)
            .with_associated_input(static_lm_lss_dist_sol)
            .with_associated_input(static_lm_lss_wave_guess)
            # .with_associated_input(static_lm_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_recipe('metis_LM_lss_std')
            .with_meta_targets([SCIENCE])
            .build())

sci_reduction_task = (task('metis_LM_lss_sci')
            .with_main_input(lm_raw_science)
            # .with_associated_input(linearity_task)
            .with_associated_input(static_badpix_map_h2rg)
            .with_associated_input(static_gain_map_h2rg)
            .with_associated_input(static_linearity_h2rg)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_associated_input(flatfield_task)
            .with_associated_input(trace_finding_task)
            # .with_associated_input(slitloss_task)
            .with_associated_input(wavelength_calibration_task)
            .with_associated_input(static_lm_lss_dist_sol)
            .with_associated_input(static_lm_lss_wave_guess)
            .with_associated_input(std_reduction_task)
            .with_associated_input(static_atm_line_cat)
            # .with_associated_input(static_lm_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_recipe('metis_LM_lss_sci')
            .with_meta_targets([SCIENCE])
            .build())

# Include branch to let user decide whether science of std data as input --> ask Lodo for branching!
mf_model_task = (task("metis_LM_lss_mf_model")
            .with_main_input(sci_reduction_task)
            .with_associated_input(static_ao_psf_model)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_LM_lss_mf_model")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_calctrans_task = (task("metis_LM_lss_calctrans")
            .with_main_input(mf_model_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_LM_lss_calctrans")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_correct_task = (task("metis_LM_lss_mf_correct")
            .with_main_input(sci_reduction_task)
            .with_associated_input(mf_calctrans_task)
            .with_recipe("metis_LM_lss_mf_correct")
            .with_meta_targets([QC1_CALIB])
            .build())




