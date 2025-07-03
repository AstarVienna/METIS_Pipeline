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

import cpl

from pymetis.classes.mixins import BandNMixin
from pymetis.classes.prefab.lss.rsrf import MetisLssRsrfImpl
from pymetis.classes.recipes import MetisRecipe


class MetisNLssRsrfImpl(MetisLssRsrfImpl):
    class InputSet(BandNMixin, MetisLssRsrfImpl.InputSet):
        pass



# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisNLssRsrf(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_rsrf"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Create spectroscopic relative spectral response function (RSRF) for the GEO detector"
    _description: str = """\
    Create relative spectral response function for the GEO LSS detector

    Inputs
        N_LSS_RSRF_RAW: Raw RSRF images [1-n]
        N_WCU_OFF_RAW:  raw WCU OFF background frames [1-n]
        MASTER_DARK_GEO: Master dark frame [optional?]
        BADPIX_MAP_GEO:  Bad-pixel map for GEO detector [optional]
        PERSISTENCE_MAP: Persistence map [optional]
        GAIN_MAP_GEO:    Gain map for GEO detector
        LINEARITY_GEO:   Linearity map for GEO detector

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        MASTER_N_LSS_RSRF:     Master flat (RSRF) frame
        MEDIAN_N_LSS_RSRF_IMG: Median map (QC)
        MEAN_N_LSS_RSRF_IMG:   Mean map (QC)
    """
# TODO: Check whether WCU_OFF frames are necessary as input (cf. ifu rsrf recipe)

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD***""" # TODO: Write description

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
# Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO: Implement real parameters
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("average", "median"),
        ),
    ])     # no parameters defined in DRLD
# Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    implementation_class = MetisNLssRsrfImpl

