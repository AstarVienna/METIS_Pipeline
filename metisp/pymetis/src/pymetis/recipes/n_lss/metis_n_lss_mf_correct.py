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
from cpl.core import Msg

from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import RawInput
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.classes.products import PipelineProduct,PipelineTableProduct

# TODO: Check 2D input spectra - correct all row with same trans?


# =========================================================================================
#    Define main class
# =========================================================================================
class MetisNLssMfCorrectImpl(RawImageProcessor):
    class InputSet(PipelineInputSet):
        band = "N"
        detector = "GEO"

    # ++++++++++++ Main input++++++++++++
        class NLssSciFlux1d(SinglePipelineInput):
            """
            Science spectrum
            """
            _tags: re.Pattern = re.compile(r"N_LSS_SCI_FLUX_1D")
            # TODO: Check the FrameGroup! Should probably PRODUCT, but a CPL error "Data not found error: Data not found" occurs if set (cf. https://www.eso.org/sci/software/pycpl/pycpl-site/api/ui.html#cpl.ui.Frame.group)
            # For the SKEL this is set to CALIB,although not correct!
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "N LSS sci flux 1D"
            _description: str = "Flux calibrated 1D N LSS science spectrum"

        class Transmission(SinglePipelineInput):
            """
            Transmission spectrum
            """
            _tags: re.Pattern = re.compile(r"N_LSS_SYNTH_TRANS")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Transmission spectrum"
            _description: str = "Transmission spectrum to be used for the telluric correction."

    # ++++++++++++++++++ Final products ++++++++++++++++++
    # TODO: Check whether calctrans creates the transmission file directly, so it should not be defined here
    class ProductTellCorrFinalSpectrum(PipelineTableProduct):
        """
        Final telluric corrected science spectrum
        """
        _tag = rf"N_LSS_SCI_FLUX_TELLCORR_1D"
        _title: str = "Final science spectrum"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description: str = "Final telluric corrected, flux + wavelength calibrated 1d science spectrum"
# =========================================================================================
#    Methods
# =========================================================================================

#   Method for processing
    def process(self) -> set[DataItem] -> [PipelineProduct]:
        """Create dummy file (should do something more fancy in the future)"""

        # TODO: Invoke mf_correct here

        # TODO: Check whether calctrans creates the Transmission file - if so, no need to
        # write it out here again
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return [
            self.ProductTellCorrFinalSpectrum(header, table),
        ]

# =========================================================================================
#    MAIN PART
# =========================================================================================

# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisNLssMfCorrect(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_mf_correct"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Application of the telluric correction"
    _description: str = """\
    Application of the telluric correction

    Inputs
        N_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science
        N_LSS_SYNTH_TRANS: Synthetic transmission of the Earth's atmosphere'

     Matched Keywords
        DRS.SLIT

    Outputs
        N_LSS_SCI_FLUX_TELL_1D: Coadded, wavelength + flux calibrated, telluric corrected 1D spectrum of the science object
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
    implementation_class = MetisNLssMfCorrectImpl
