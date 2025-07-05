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

from pymetis.classes.mixins import BandLmMixin
from pymetis.classes.prefab.lss.sci import MetisLssSciImpl
from pymetis.classes.recipes import MetisRecipe


class MetisLmLssSciImpl(MetisLssSciImpl):
    class InputSet(BandLmMixin, MetisLssSciImpl.InputSet):
        pass


class MetisLmLssSci(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_sci"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Reduction of the LSS science star frames"
    _description: str = """\
    Reduction of the LSS science star frames

    Inputs
        LM_LSS_SCI_RAW:     Raw science images [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        LINEARITY_2RG:      Linearity map for 2RG detector
        GAIN_MAP_2RG:       Gain map for 2RG detector
        BADPIX_MAP_2RG:     Bad-pixel map for 2RG detector [optional]
        MASTER_DARK_2RG:    Master dark frame [optional?]
        MASTER_LM_LSS_RSRF: Master flat (RSRF) frame
        LM_LSS_DIST_SOL:    Distortion solution
        LM_LSS_WAVE_GUESS:  First guess of the wavelength solution
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        LM_ADC_SLITLOSS:    Slitloss information
        STD_TRANSMISSION:   Transmission of the Earth's atmosphere derived from the STD for telluric correction  [optional]
        MASTER_LM_RESPONSE: Response function for flux calibration

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        LM_LSS_SCI_OBJ_MAP: Pixel map of the object pixels (QC)
        LM_LSS_SCI_SKY_MAP: Pixel map of the sky pixels (QC)
        LM_LSS_SCI_2D:      Coadded, wavelength calibrated, 2D spectrum of the science object
        LM_LSS_SCI_1D:      Coadded, wavelength calibrated, collapsed 1D spectrum of the science object
        LM_LSS_SCI_FLUX_2D: Coadded, wavelength + flux calibrated, 2D spectrum of the science object
        LM_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object
        LM_LSS_SCI_FLUX_TELL_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object (optional)
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
    implementation_class = MetisLmLssSciImpl
