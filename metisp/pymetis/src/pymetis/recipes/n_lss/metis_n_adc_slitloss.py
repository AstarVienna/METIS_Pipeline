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

import re
import cpl
from cpl.core import Msg

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor
from pymetis.classes.inputs import RawInput
from pymetis.classes.dataitems.adc.adc import NAdcSlitloss

# =========================================================================================
#    Define main class
# =========================================================================================
class MetisNAdcSlitlossImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):   # <---- TODO: need to give more here?
        # Define input classes ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        class RawInput(RawInput):
            """
            Raw image N_LSS_SLITLOSS_RAW
            """
            _tags: re.Pattern = re.compile(r"N_ADC_SLITLOSS_RAW")   # <---- TBD
            _title: str = "N ADC slitloss raw"
            _description: str = "Raw files for ADC slitloss determination (TBD)."
        class NAdcSlitlossWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"N_WCU_OFF_RAW")
            _title: str = "N LSS WCU off"
            _description: str = "Raw data for dark subtraction in other recipes."

    ProductNAdcSlitloss = NAdcSlitloss


# =========================================================================================
#    Methods
# =========================================================================================


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

#   Method for processing
    def process_images(self):
        """Create dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return {
            self.ProductNAdcSlitloss(header, table),
        }


# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisNAdcSlitloss(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
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
    implementation_class = MetisNAdcSlitlossImpl
