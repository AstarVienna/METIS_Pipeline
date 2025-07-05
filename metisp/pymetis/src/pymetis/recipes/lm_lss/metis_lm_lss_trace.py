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
from pymetis.classes.prefab.lss.trace import MetisLssTraceImpl
from pymetis.classes.recipes import MetisRecipe


class MetisLmLssTraceImpl(MetisLssTraceImpl):
    class InputSet(BandNMixin, MetisLssTraceImpl.InputSet):
        pass

# =========================================================================================
#    MAIN PART
# =========================================================================================

# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisLmLssTrace(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_trace"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Detection of LM order location on the 2RG detector"
    _description: str = """\
    Detection of LM order location on the 2RG detector

    Inputs
        LM_LSS_RSRF_PINH_RAW: Raw RSRF pinhole frames [1-n]
        LM_WCU_OFF_RAW:       Raw WCU OFF background frames [1-n]
        PERSISTENCE_MAP:      Persistence map [optional]
        GAIN_MAP_2RG:         Gain map for 2RG detector
        LINEARITY_2RG:        Linearity map for 2RG detector
        MASTER_DARK_2RG:      Master dark frame [optional?]
        BADPIX_MAP_2RG:       Bad-pixel map for 2RG detector [optional]
        MASTER_LM_LSS_RSRF:   Master flat (RSRF) frame

    Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        LM_LSS_TRACE:   Location of the orders ***TBD***
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD*** """

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO: Implement real parameters
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}parameter1",
            context=_name,
            description="Description of parameter 1",
            default="value1",
            alternatives=("value2", "value1"),
        ),
    ])
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    implementation_class = MetisLmLssTraceImpl

