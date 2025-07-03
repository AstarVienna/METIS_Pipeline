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
# from typing import Any, Dict   # <------ TODO: Check whether necessary, taken from example of pyesorex webpages


# TODO: Check the need for WCU_OFF frames!

# Import the required PyCPL modules
import re
import cpl
from cpl.core import Msg

from pymetis.classes.mixins import Detector2rgMixin

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor

from pymetis.classes.recipes.impl import MetisRecipeImpl
from pymetis.classes.inputs import (BadpixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin)
from pymetis.classes.products import PipelineTableProduct

# =========================================================================================
#    Define main class
# =========================================================================================
class MetisLmAdcSlitlossImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):   # <---- TODO: need to give more here?
        band = "LM"    # <---- TODO: Check why not automatically determined
        detector = "2RG"   # <---- TODO: Check why not automatically determined

        # Define input classes ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        class RawInput(RawInput):
            """
            Raw image LM_LSS_SLITLOSS_RAW
            """
            _tags: re.Pattern = re.compile(r"LM_ADC_SLITLOSS_RAW")   # <---- TBD
            _title: str = "LM ADC slitloss raw"
            _description: str = "Raw files for ADC slitloss determination (TBD)."
        class LmAdcSlitlossWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")
            _title: str = "LM LSS WCU off"
            _description: str = "Raw data for dark subtraction in other recipes."


    # ++++++++++++++++++ Final products ++++++++++++++++++
    class ProductLmAdcSlitloss(PipelineTableProduct):
        """
        Final Master RSRF
        """
        _tag: str = r"LM_ADC_SLITLOSS"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Table with ADC induced LM slitlosses"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)


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
    def process_images(self) -> [PipelineTableProduct]:
        """Create dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return [
            self.ProductLmAdcSlitloss(self, header, table),
        ]


# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe
class MetisLmAdcSlitloss(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_adc_slitloss"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Determines ADC slitlosses"
    _description: str = """\
    Determines ADC slitlosses

    Remark: Recipe not welldefined as actual algorithm not well defined (cf. DRLD, Calib plan)

    Inputs
        LM_ADC_SLITLOSS_RAW: Raw SLITLOSS images [1-n]  ***TBD***
        LM_WCU_OFF_RAW:      Raw WCU OFF background frames [1-n]
        MASTER_DARK_2RG:     Master dark frame [optional?]  ***TBChecked***
        BADPIX_MAP_2RG:      Bad-pixel map for 2RG detector [optional] ***TBChecked***
        PERSISTENCE_MAP:     Persistence map [optional] ***TBChecked***
        GAIN_MAP_2RG:        Gain map for 2RG detector ***TBChecked***
        LINEARITY_2RG:       Linearity map for 2RG detector ***TBChecked***

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        LM_ADC_SLITLOSS:     Table with slit losses ***TBD***
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
    implementation_class = MetisLmAdcSlitlossImpl

