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

from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.lss.curve import LssCurve, LssDistSol, LssWaveGuess
from pymetis.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.dataitems.lss.trace import LssTrace
from pymetis.dataitems.lss.wave import LssWaveRaw
from pymetis.classes.inputs import (SinglePipelineInput, RawInput,
                                    LaserTableInput,
                                    PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin,
                                    LinearityInputSetMixin)
from pymetis.classes.inputs.common import WcuOffInput
from pymetis.classes.mixins import BandLmMixin
from pymetis.classes.prefab import DarkImageProcessor
from pymetis.classes.recipes import MetisRecipe
from pymetis.utils.dummy import create_dummy_table, create_dummy_header


class MetisLmLssWaveImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin,
                   BandLmMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssWaveRaw

        class MasterRsrfInput(SinglePipelineInput):
            Item = MasterLssRsrf

        class LssTraceInput(SinglePipelineInput):
            Item = LssTrace

        WcuOffInput = WcuOffInput
        LaserTableInput = LaserTableInput

    # ++++++++++++ Intermediate / QC products ++++++++++++
    ProductLssCurve = LssCurve
    # ++++++++++++++++++ Final products ++++++++++++++++++
    ProductLssDistSol = LssDistSol
    ProductLssWaveGuess = LssWaveGuess

#   Method for processing
    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""
        raws = self.inputset.raw.load_list()
        master_lss_rsrf = self.inputset.master_rsrf.load_data()

        header = master_lss_rsrf.header
        table = create_dummy_table()

        LmLssCurveHdr = header
        LmLssDistSolHdr = header
        LmLssWaveGuessHdr = header

        return {
            self.ProductLssCurve(LmLssCurveHdr, table),
            self.ProductLssDistSol(LmLssDistSolHdr, table),
            self.ProductLssWaveGuess(LmLssWaveGuessHdr, table),
        }


# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisLmLssWave(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_wave"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "First guess of the wavelength solution based on WCU laser reference"
    _description: str = """\
    First guess of the wavelength solution based on WCU laser reference

    Inputs
        LM_LSS_WAVE_RAW:    Raw WCU laser spectra [1-n]
        LM_WCU_OFF_RAW:     Raw WCU OFF background frames [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        GAIN_MAP_2RG:       Gain map for 2RG detector
        LINEARITY_2RG:      Linearity map for 2RG detector
        MASTER_DARK_2RG:    Master dark frame [optional?]
        BADPIX_MAP_2RG:     Bad-pixel map for 2RG detector [optional]
        MASTER_LM_LSS_RSRF: Master flat (RSRF) frame
        LM_LSS_TRACE:       Location of the orders (TBD)
        LASER_TAB:          Table with laser lines

    Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        LM_LSS_CURVE:      Line curvature table (TBD)
        LM_LSS_DIST_SOL:   Distortion solution
        LM_LSS_WAVE_GUESS: First guess of the wavelength solution
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
    Impl = MetisLmLssWaveImpl

