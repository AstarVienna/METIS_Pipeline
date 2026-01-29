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
"""

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.mixins import BandLmMixin
from pymetis.classes.prefab.img.std_process import MetisImgStdProcessImpl
from pymetis.classes.qc import QcParameterSet
from pymetis.classes.recipes import MetisRecipe
from pymetis.dataitems.background.subtracted import LmStdBackgroundSubtracted
from pymetis.qc.std_process import (QcImgStdBackgroundRms, QcStdPeakCounts, QcStdApertureCounts, QcStdStrehl,
                                    QcStdFwhm, QcStdEllipticity, QcStdFluxConversion, QcSensitivity, QcAreaSensitivity)


class MetisLmImgStdProcessImpl(BandLmMixin, MetisImgStdProcessImpl):
    class InputSet(MetisImgStdProcessImpl.InputSet):
        class RawInput(MetisImgStdProcessImpl.InputSet.RawInput):
            Item = LmStdBackgroundSubtracted

    class Qc(QcParameterSet):
        BackgroundRms = QcImgStdBackgroundRms
        PeakCounts = QcStdPeakCounts
        ApertureCounts = QcStdApertureCounts
        Strehl = QcStdStrehl
        Fwhm = QcStdFwhm
        Ellipticity = QcStdEllipticity
        FluxConversion = QcStdFluxConversion
        Sensitivity = QcSensitivity
        AreaSensitivity = QcAreaSensitivity


class MetisLmImgStdProcess(MetisRecipe):
    _name: str = "metis_lm_img_std_process"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine the conversion factor between detector counts and physical source flux"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm: str = """Call metis_lm_calculate_std_flux to measure flux in input images
        call hdrl_resample_compute to recenter the images
        call hdrl_imagelist_collapse to stack the images
        call metis_lm_calculate_std_flux on the stacked image to get flux of the star in detector units
        call metis_calculate_std_fluxcal to calculate the conversion factor to physical units
        call metis_calculate_detection_limits to compute background noise (std, rms) and compute detection limits
    """

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisLmImgStdProcessImpl
