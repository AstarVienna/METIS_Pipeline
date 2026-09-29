"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The observing templates of METIS, as the DRLD names them: the vocabulary `Recipe._templates`
draws from. The pipeline never reads TPL.ID (classification runs on the DPR keywords, the
workflows group on TPL.START), so these are documentation: every recipe's declaration must
be one of them, and every one of them must be claimed by some recipe, which is what catches a
typo or a template pasted onto the wrong recipe. Grow the list as templates are defined.
"""

TEMPLATES: frozenset[str] = frozenset({
    # detectors
    "METIS_gen_cal_dark", "METIS_gen_cal_InsDark",
    "METIS_img_lm_cal_DetLin", "METIS_img_n_cal_DetLin", "METIS_ifu_cal_DetLin",
    # LM imaging
    "METIS_img_lm_cal_ChopperHome", "METIS_img_lm_cal_InternalFlat", "METIS_img_lm_cal_TwilightFlat",
    "METIS_img_lm_cal_distortion", "METIS_img_lm_cal_standard", "METIS_img_lm_cal_psf",
    "METIS_img_lm_obs_AutoJitter", "METIS_img_lm_obs_GenericOffset", "METIS_img_lm_obs_FixedSkyOffset",
    "METIS_img_lm_app_obs_FixedOffset", "METIS_img_lm_vc_obs_FixedSkyOffset",
    "METIS_lm_img_calibrate", "METIS_lm_img_std_process",
    # N imaging
    "METIS_img_n_cal_InternalFlat", "METIS_img_n_cal_TwilightFlat", "METIS_img_n_cal_distortion",
    "METIS_img_n_cal_standard", "METIS_img_n_cal_psf",
    "METIS_img_n_obs_AutoChopNod", "METIS_img_n_obs_GenericChopNod", "METIS_img_n_cvc_obs_AutoChop",
    "METIS_n_img_calibrate", "METIS_n_img_std_process",
    # both imagers
    "METIS_img_lmn_obs_AutoChopNod", "METIS_img_lmn_obs_GenericChopNod",
    # IFU
    "METIS_ifu_cal_distortion", "METIS_ifu_cal_rsrf", "METIS_ifu_cal_InternalWave",
    "METIS_ifu_cal_psf", "METIS_ifu_cal_standard",
    "METIS_ifu_obs_FixedSkyOffset", "METIS_ifu_obs_GenericOffset",
    "METIS_ifu_ext_obs_FixedSkyOffset", "METIS_ifu_ext_obs_GenericOffset",
    "METIS_ifu_vc_obs_FixedSkyOffset", "METIS_ifu_ext_vc_obs_FixedSkyOffset",
    "METIS_ifu_app_obs_Stare", "METIS_ifu_ext_app_obs_Stare",
})
