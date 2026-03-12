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

# TODO: Check the need for WCU_OFF frames!
from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.mixins import BandNMixin
from pymetis.recipes.prefab.lss.adc import MetisAdcSlitlossImpl
from pymetis.classes.recipes import MetisRecipe


class MetisNAdcSlitlossImpl(MetisAdcSlitlossImpl):
    class InputSet(BandNMixin, MetisAdcSlitlossImpl.InputSet):
        pass


class MetisNAdcSlitloss(MetisRecipe):
    _name: str = "metis_n_adc_slitloss"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Determines ADC slitlosses"
    _description: str = """\
    Determines ADC slitlosses

    Remark: Recipe not welldefined as actual algorithm not well defined (cf. DRLD, Calib plan)

    Inputs
        N_ADC_SLITLOSS_RAW:  Raw SLITLOSS images [1-n]  ***TBD***
        N_WCU_OFF_RAW:       Raw WCU OFF background frames [1-n]
        MASTER_DARK_GEO:     Master dark frame [optional?]  ***TBChecked***
        BADPIX_MAP_GEO:      Bad-pixel map for GEO detector [optional] ***TBChecked***
        PERSISTENCE_MAP:     Persistence map [optional] ***TBChecked***
        GAIN_MAP_GEO:        Gain map for GEO detector ***TBChecked***
        LINEARITY_GEO:       Linearity map for GEO detector ***TBChecked***

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        N_ADC_SLITLOSS:     Table with slit losses ***TBD***
    """
# TODO: Check whether WCU_OFF frames are necessary as input (cf. ifu rsrf recipe)

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Incredible fancy description of algorithm follows... ***TBD***""" # TODO: Write description

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
    Impl = MetisNAdcSlitlossImpl
