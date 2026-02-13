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

from pymetis.classes.mixins import BandLmMixin, Detector2rgMixin
from pymetis.classes.prefab.lss.adc import MetisAdcSlitlossImpl
from pymetis.classes.recipes import MetisRecipe


class MetisLmAdcSlitlossImpl(BandLmMixin, Detector2rgMixin, MetisAdcSlitlossImpl):
    class InputSet(MetisAdcSlitlossImpl.InputSet):
        pass


class MetisLmAdcSlitloss(MetisRecipe):
    _name: str = "metis_lm_adc_slitloss"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Determines ADC slitlosses"
    # TODO: Check whether WCU_OFF frames are necessary as input (cf. ifu rsrf recipe)

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
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
    Impl = MetisLmAdcSlitlossImpl

