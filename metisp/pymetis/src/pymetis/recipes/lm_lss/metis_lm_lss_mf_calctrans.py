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


# Import the required PyCPL modules
import re
import cpl

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.synth import SynthTrans, LmLssSynthTrans
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet

# =========================================================================================
#    Define main class
# =========================================================================================
class MetisLmLssMfCalctransImpl(RawImageProcessor):
    class InputSet(PipelineInputSet):
        class MfBestFitTab(SinglePipelineInput):
            """
            Table with best-fit parameters
            """
            _tags: re.Pattern = re.compile(r"MF_BEST_FIT_TAB")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Table with best-fit parameters"
            _description: str = "Calculation of transmission function."

        class LsfKernel(SinglePipelineInput):
            """
            Table with LSF Kernel (if external kernel is used)
            """
            _tags: re.Pattern = re.compile(r"LSF_KERNEL")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "LSF Kernel"
            _description: str = "Kernel of the Line-Spred-Function."

        class AtmLineCat(SinglePipelineInput):
            """
            Atmospheric line catalogue
            """
            _tags: re.Pattern = re.compile(r"ATM_LINE_CAT")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Table with best-fit parameters"
            _description: str = "Line catalogue of atmopsheric lines."

        class AtmProfile(SinglePipelineInput):
            """
            Atmospheric input profile
            """
            _tags: re.Pattern = re.compile(r"ATM_PROFILE")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Table with best-fit parameters"
            _description: str = "Atmospheric profile."

    ProductTransmission = LmLssSynthTrans
# =========================================================================================
#    Methods
# =========================================================================================

#   Method for processing
    def process(self) -> set[DataItem]:
        """Create dummy file (should do something more fancy in the future)"""

        # TODO: Invoke mf_calctrans here

        # TODO: Check whether calctrans creates the Transmission file - if so, no need to
        # write it out here again
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return {
            self.ProductTransmission(header, table),
        }


# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisLmLssMfCalctrans(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_mf_calctrans"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Calculation of transmission function"
    _description: str = """\
    Calculation of transmission function

    Inputs
        MF_BEST_FIT_TAB: Table with best-fit parameters
        LSF_KERNEL:      LSF Kernel file
        ATM_LINE_CAT:    Catalogue of atmospheric lines
        ATM_PROFILE:     Atmospheric input profile

     Matched Keywords
        DRS.SLIT

    Outputs
        LM_LSS_SYNTH_TRANS: Synthetic transmission of the Earth's atmosphere
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD***"""

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
    implementation_class = MetisLmLssMfCalctransImpl
