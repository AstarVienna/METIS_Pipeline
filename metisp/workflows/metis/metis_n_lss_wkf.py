# METIS LSS N BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: V0.1 (Skeleton level)
#
#   TODO: Next steps: Implement telluric correction branches
#
#

"""METIS LSS N-Band workflow"""
from edps import SCIENCE, QC1_CALIB, QC0, CALCHECKER
from edps import subworkflow, task, data_source, classification_rule
from .metis_classification import *
from .metis_datasources import *
from .metis_task_functions import *
from .metis_n_lss_molecfit import telluric_on_standard, telluric_on_science

# ----------------------------------------------------------------------------
# ------------------------ Processing tasks ----------------------------------
# ----------------------------------------------------------------------------

# persistence_task = (task('metis_det_persistence')
#             .with_main_input(raw_slitloss)
#             .with_recipe("metis_det_persistence")
#             .build())

n_lss_lingain_task = (task('metis_n_lss_lingain')
            .with_recipe("metis_det_lingain")
            .with_main_input(detlin_geo_raw)
            .with_associated_input(n_wcu_off_raw)        # check how that is exactly delivered by the ICS!
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_dark_task = (task('metis_n_lss_dark')
            .with_recipe("metis_det_dark")
            .with_main_input(dark_geo_raw)       # check what the minimum number of raw darks is!
            .with_associated_input(persistence_map)
            .with_associated_input(n_lss_lingain_task)
            .with_meta_targets([QC1_CALIB])
            .build())

n_adc_slitloss_task = (task('metis_n_adc_slitloss')
            .with_recipe("metis_n_adc_slitloss")
            .with_main_input(n_adc_slitloss_raw)
            .with_associated_input(n_wcu_off_raw)        # check how that is exactly delivered by the ICS!
            .with_associated_input(persistence_map)
            .with_associated_input(n_lss_lingain_task) # Shouldn't this also need a MASTER_IMG_FLAT_LAMP_N as per the DRLD?
            .with_associated_input(n_lss_dark_task)
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_rsrf_task = (task('metis_n_lss_rsrf')
            .with_recipe("metis_n_lss_rsrf")
            .with_main_input(n_lss_rsrf_raw)
            .with_associated_input(n_wcu_off_raw)
            .with_associated_input(n_lss_dark_task)
            .with_associated_input(persistence_map)
            .with_associated_input(n_lss_lingain_task)
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_trace_task = (task("metis_n_lss_trace")
            .with_recipe("metis_n_lss_trace")
            .with_main_input(n_lss_rsrf_pinh_raw)
            .with_associated_input(n_wcu_off_raw)
            .with_associated_input(n_lss_dark_task)
            .with_associated_input(n_lss_lingain_task)
            .with_associated_input(n_lss_rsrf_task)
            .with_associated_input(persistence_map)
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_std_task = (task('metis_n_lss_std')
            .with_main_input(n_lss_std_raw)
            # .with_associated_input(n_lss_lingain_task)
            .with_associated_input(badpix_map_geo)
            .with_associated_input(gain_map_geo)
            .with_associated_input(linearity_geo)
            .with_associated_input(persistence_map)
            .with_associated_input(n_lss_dark_task)
            .with_associated_input(n_lss_rsrf_task)
            .with_associated_input(n_lss_trace_task)
            # .with_associated_input(wavelength_calibration_task)
            .with_associated_input(n_adc_slitloss_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(ref_std_cat)
            .with_associated_input(n_lss_synth_trans)
            .with_associated_input(n_lss_dist_sol)
            .with_associated_input(n_lss_wave_guess)
            .with_associated_input(n_adc_slitloss)
            .with_associated_input(ao_psf_model)
            .with_recipe('metis_n_lss_std')
            .with_meta_targets([SCIENCE])
            .build())

n_lss_sci_task = (task('metis_n_lss_sci')
            .with_main_input(n_lss_sci_raw)
            # .with_associated_input(n_lss_lingain_task)
            .with_associated_input(badpix_map_geo)
            .with_associated_input(gain_map_geo)
            .with_associated_input(linearity_geo)
            .with_associated_input(persistence_map)
            .with_associated_input(n_lss_dark_task)
            .with_associated_input(n_lss_rsrf_task)
            .with_associated_input(n_lss_trace_task)
            # .with_associated_input(wavelength_calibration_task)
            .with_associated_input(n_lss_dist_sol)
            .with_associated_input(n_lss_wave_guess)
            .with_associated_input(n_lss_std_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(n_adc_slitloss)
            .with_associated_input(ao_psf_model)
            .with_recipe('metis_n_lss_sci')
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
n_lss_mf_model_task = (task("metis_n_lss_mf_model")
            .with_main_input(n_lss_std_task)
            # .with_associated_input(static_ao_psf_model)
            .with_associated_input(atm_line_cat)
            .with_associated_input(lsf_kernel)
            .with_associated_input(atm_profile)
            # TODO: Include output filtering for sci & std outputs
            .with_associated_input(n_lss_std_task, condition=on_standard)
            .with_associated_input(n_lss_sci_task, condition=on_science)
            .with_recipe('metis_n_lss_mf_model')
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_mf_calctrans_task = (task("metis_n_lss_mf_calctrans")
            .with_main_input(n_lss_mf_model_task)
            .with_associated_input(atm_line_cat)
            .with_associated_input(lsf_kernel)
            .with_associated_input(atm_profile)
            .with_recipe('metis_n_lss_mf_calctrans')
            .with_meta_targets([QC1_CALIB])
            .build())

n_lss_mf_correct_task = (task("metis_n_lss_mf_correct")
            .with_main_input(n_lss_sci_task)
            .with_associated_input(n_lss_mf_calctrans_task)
            .with_recipe("metis_n_lss_mf_correct")
            .with_meta_targets([QC1_CALIB, SCIENCE])
            .build())




