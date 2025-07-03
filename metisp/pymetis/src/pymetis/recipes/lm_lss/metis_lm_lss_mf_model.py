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

from pymetis.classes.inputs import (SinglePipelineInput, PipelineInputSet,
                                    AtmProfileInput, AtmLineCatInput, LsfKernelInput)
from pymetis.classes.prefab.rawimage import RawImageProcessor


# =========================================================================================
#    Define main class
# =========================================================================================
class MetisLmLssMfModelImpl(RawImageProcessor):
    class InputSet(PipelineInputSet):
    # ++++++++++++ Main input ++++++++++++
        # Default (Path #2 in DRLD Section CritAlg)
        class LmLssSciFlux1d(SinglePipelineInput):
            """
            Science spectrum
            """
            _tags: re.Pattern = re.compile(r"LM_LSS_SCI_FLUX_1D")
            # TODO: Check the FrameGroup! Should probably PRODUCT, but a CPL error "Data not found error: Data not found" occurs if set (cf. https://www.eso.org/sci/software/pycpl/pycpl-site/api/ui.html#cpl.ui.Frame.group)
            # For the SKEL this is set to CALIB,although not correct!
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "LM LSS sci flux 1D"
            _description: str = "Flux calibrated 1D LM LSS science spectrum"

        # Alternative (Path #3 in DRLD Section CritAlg)
        class LmLssStdFlux1d(SinglePipelineInput):
            """
            Standard star spectrum
            """
            _tags: re.Pattern = re.compile(r"LM_LSS_STD_1D")
            # TODO: Check the FrameGroup! Should probably PRODUCT, but a CPL error "Data not found error: Data not found" occurs if set (cf. https://www.eso.org/sci/software/pycpl/pycpl-site/api/ui.html#cpl.ui.Frame.group)
            # For the SKEL this is set to CALIB,although not correct!
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "LM LSS standard star 1D spectrum"
            _description: str = "1D LM LSS standard star spectrum"
    # ++++++++++++ Intermediate / QC products ++++++++++++
    # Currently none foreseen (some for QC?)

        AtmProfileInput = AtmProfileInput
        AtmLineCatInput = AtmLineCatInput
        LsfKernelInput = LsfKernelInput

    ProductMfBestFitTable = MfBestFitTable



# =========================================================================================
#    Methods
# =========================================================================================

#   Method for processing
    def process(self) -> set[DataItem]:
        """Create dummy file (should do something more fancy in the future)"""

        # TODO: Invoke molecfit here
        # TODO: Check whether the new mf writes out the best-fit param file
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return {
            self.ProductMfBestFitTab(header, table),
        }


#   Method for loading images (stolen from metis_chop_home.py)
    def load_images(self, frameset: cpl.ui.FrameSet) -> cpl.core.ImageList:
        """Load an imagelist from a FrameSet

        This is a temporary implementation that should be generalized to the
        entire pipeline package. It uses cpl functions - these should be
        replaced with hdrl functions once they become available, in order
        to use uncertainties and masks.
        """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(frameset):
            Msg.info(self.__class__.__qualname__,
                     f"Processing input frame #{idx}: {frame.file!r}...")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output

# =========================================================================================
#    MAIN PART
# =========================================================================================

# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisLmLssMfModel(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_mf_model"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Calculation of molecfit model"
    _description: str = """\
    Calculation of molecfit model

    Inputs
        LM_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object
        LM_LSS_STD_1D: Coadded, wavelength calibrated, collapsed 1D spectrum of the standard star
        LSF_KERNEL:         LSF Kernel file
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        ATMP_PROFILE:       Atmospheric input profile

     Matched Keywords
        DRS.SLIT    ***TBChecked***

    Outputs
        MF_BEST_FIT_TAB: Table with best-fit parameters
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
    implementation_class = MetisLmLssMfModelImpl
