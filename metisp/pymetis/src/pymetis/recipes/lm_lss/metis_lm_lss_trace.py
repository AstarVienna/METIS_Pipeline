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

from pymetis.classes.mixins.detector import Detector2rgMixin
from pymetis.classes.prefab import DarkImageProcessor

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import (SinglePipelineInput, BadpixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin, PersistenceInputSetMixin)
from pymetis.classes.products import PipelineTableProduct

# =========================================================================================
#    Define main class
# =========================================================================================
class MetisLmLssTraceImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        band = "LM"
        detector = "2RG"

        """
        Raw pinhole frames LM_LSS_RSRF_PINH_RAW
        """
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_LSS_RSRF_PINH_RAW")
            _title: str = "LM LSS rsrf pinhole raw"
            _description: str = "Raw flats taken with black-body calibration lamp through the pinhole mask."

        """
        WCU off frames
        """
        class LmRsrfWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")
            _title: str = "LM LSS WCU off"
            _description: str = "Raw data for dark subtraction in other recipes."

        """
        Master dark MASTER_DARK_2RG
        """
        class MasterDarkInput(MasterDarkInput):
            _tags: re.Pattern = re.compile(r"MASTER_DARK_2RG")

        """
        Bad pixel BADPIX_MAP_2RG
        """
        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _tags: re.Pattern = re.compile(r"BADPIX_MAP_2RG")

        """
        Gain map
        """
        class GainMapInput(GainMapInput):
            _tags: re.Pattern = re.compile(r"GAIN_MAP_2RG")

        """
        Linearity
        """
        class LinearityInput(LinearityInput):
            _tags: re.Pattern = re.compile(r"LINEARITY_2RG")

        """
        MASTER LM LSS RSRF
        """
        class MasterRsrfInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"MASTER_LM_LSS_RSRF")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "MASTER_RSRF"
            _description: str = "Master 2D RSRF"


    # ++++++++++++++++++ Final products ++++++++++++++++++
    """
    Final trace table
    """
    class ProductTraceTab(PipelineTableProduct):
        _tag: str = r"LM_LSS_TRACE"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Table with polynomials describing the location of the traces on the detector"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


# =========================================================================================
#    Methods
# =========================================================================================

    """
    Method for loading images (stolen from metis_chop_home.py)
    """
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

    """
    Method for processing
    """
    def process_images(self) -> [PipelineTableProduct]:
        """Create dummy file (should do something more fancy in the future)"""
        # trace_tab_hdr = self._create_dummy_header()
        trace_tab_hdr = self._create_dummy_header()
        trace_tab_data = self._create_dummy_table()
        return [
            self.ProductTraceTab(self, trace_tab_hdr, trace_tab_data)
        ]

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
    _undescription: str = """\
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
    """
    Define parameters
    """
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

