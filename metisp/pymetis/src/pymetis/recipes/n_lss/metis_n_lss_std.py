"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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

from pymetis.classes.mixins import BandNMixin
from pymetis.classes.prefab.lss.std import MetisLssStdImpl
from pymetis.classes.recipes import MetisRecipe


class MetisNLssStdImpl(MetisLssStdImpl):
    class InputSet(BandNMixin, MetisLssStdImpl.InputSet):
        pass


class MetisNLssStd(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_std"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Reduction of the standard star frames for determining the response function (flux calibration) and/or the transmission (telluric correction)"
    _description: str = """\
    Reduction of the standard star frames for determining the response function (flux calibration) and/or the transmission (telluric correction)

    Inputs
        N_LSS_STD_RAW:     Raw standard star images [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        LINEARITY_GEO:      Linearity map for GEO detector
        GAIN_MAP_GEO:       Gain map for GEO detector
        BADPIX_MAP_GEO:     Bad-pixel map for GEO detector [optional]
        MASTER_DARK_GEO:    Master dark frame [optional?]
        MASTER_N_LSS_RSRF: Master flat (RSRF) frame
        N_LSS_DIST_SOL:    Distortion solution
        N_LSS_WAVE_GUESS:  First guess of the wavelength solution
        AO_PSF_MODEL:       Model of the AO PSF
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        N_ADC_SLITLOSS:    Slitloss information
        N_SYNTH_TRANS:     Synthetic model of the Earth's atmopshere transmission
        REF_STD_CAT:        Catalogue of standard stars

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        N_LSS_STD_OBJ_MAP: Pixel map of the object pixels (QC)
        N_LSS_STD_SKY_MAP: Pixel map of the sky pixels (QC)
        N_LSS_STD_1D:      Coadded, wavelength calibrated, collapsed 1D spectrum of the standard star
        N_LSS_STD_WAVE:    Wavelength solution based on std star
        STD_TRANSMISSION:   Transmission of the Earth's atmosphere derived from the STD for telluric correction  [optional]
        MASTER_N_RESPONSE: Response function for flux calibration
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD***"""

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO: Implement real parameters
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}parameter1",
            context=_name,
            description="Description of parameter 1",
            default="value1",
            alternatives=("value2", "value1"),
        ),
    ])
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    implementation_class = MetisNLssStdImpl
