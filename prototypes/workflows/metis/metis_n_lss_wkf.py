# METIS LSS N BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

"""METIS LSS N-Band workflow"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import task, data_source, classification_rule
from .metis_n_lss_classification import *
from .metis_n_lss_datasources import *
from .metis_n_lss_task_functions import *

# --- Processing tasks ---
# persistence_task = (task('metis_det_persistence')
#             .with_main_input(raw_slitloss)
#             .with_recipe("metis_det_persistence")
#             .build())

linearity_task = (task('metis_det_lingain')
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_main_input(raw_lingain)
            .with_associated_input(geo_wcu_off)
            .with_recipe("metis_det_lingain")
            .with_meta_targets([QC1_CALIB])
            .build())

slitloss_task = (task('metis_n_adc_slitloss')
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_main_input(raw_slitloss)
            .with_recipe("metis_n_adc_slitloss")
            .with_meta_targets([QC1_CALIB])
            .build())

dark_task = (task('metis_det_dark')
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_main_input(raw_dark)
            .with_recipe("metis_det_dark")
            .with_meta_targets([QC1_CALIB])
            .build())

flatfield_task = (task('metis_N_lss_rsrf')
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_main_input(n_raw_rsrf)
            .with_associated_input(dark_task)
            .with_recipe("metis_N_lss_rsrf")
            .with_meta_targets([QC1_CALIB])
            .build())

trace_finding_task = (task("metis_N_lss_trace")
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_main_input(n_raw_rsrf_pinh)
            .with_associated_input(dark_task)
            .with_recipe("metis_N_lss_trace")
            .with_meta_targets([QC1_CALIB])
            .build())

std_reduction_task = (task('metis_N_lss_std')
            .with_recipe('metis_N_lss_std')
            .with_main_input(n_raw_std)
            # .with_associated_input(linearity_task)
            .with_associated_input(static_badpix_map_geo)
            .with_associated_input(static_gain_map_geo)
            .with_associated_input(static_linearity_geo)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_associated_input(flatfield_task)
            .with_associated_input(trace_finding_task)
            .with_associated_input(slitloss_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_ref_std_cat)
            .with_associated_input(static_n_lss_dist_sol)
            .with_associated_input(static_n_lss_wave_guess)
            # .with_associated_input(static_n_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_meta_targets([SCIENCE])
            .build())

sci_reduction_task = (task('metis_N_lss_sci')
            .with_recipe('metis_N_lss_sci')
            .with_main_input(n_raw_science)
            # .with_associated_input(linearity_task)
            .with_associated_input(static_badpix_map_geo)
            .with_associated_input(static_gain_map_geo)
            .with_associated_input(static_linearity_geo)
            .with_associated_input(static_persistence_map, min_ret=0)
            .with_associated_input(dark_task)
            .with_associated_input(flatfield_task)
            .with_associated_input(trace_finding_task)
            .with_associated_input(static_n_lss_dist_sol)
            .with_associated_input(static_n_lss_wave_guess)
            .with_associated_input(std_reduction_task)
            .with_associated_input(static_atm_line_cat)
            # .with_associated_input(static_n_adc_slitloss)
            .with_associated_input(static_ao_psf_model)
            .with_meta_targets([SCIENCE])
            .build())

mf_model_task = (task("metis_N_lss_mf_model")
            .with_main_input(sci_reduction_task)
            .with_associated_input(static_ao_psf_model)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_N_lss_mf_model")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_calctrans_task = (task("metis_N_lss_calctrans")
            .with_main_input(mf_model_task)
            .with_associated_input(static_atm_line_cat)
            .with_associated_input(static_lsf_kernel)
            .with_associated_input(static_atm_profile)
            .with_recipe("metis_N_lss_calctrans")
            .with_meta_targets([QC1_CALIB])
            .build())

mf_correct_task = (task("metis_N_lss_mf_correct")
            .with_main_input(sci_reduction_task)
            .with_associated_input(mf_calctrans_task)
            .with_recipe("metis_N_lss_mf_correct")
            .with_meta_targets([QC1_CALIB])
            .build())




