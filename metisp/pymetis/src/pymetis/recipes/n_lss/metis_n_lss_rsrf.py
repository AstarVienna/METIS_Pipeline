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

# Import the required PyCPL modules
import re
import cpl
from cpl.core import Msg

from pymetis.classes.mixins import DetectorGeoMixin

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor

from pymetis.classes.recipes.impl import MetisRecipeImpl
from pymetis.classes.inputs import (BadpixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin, PersistenceInputSetMixin)
from pymetis.classes.products import PipelineImageProduct

# =========================================================================================
#    Define main class
# =========================================================================================
class MetisNLssRsrfImpl(RawImageProcessor):
    class InputSet(PersistenceInputSetMixin, RawImageProcessor.InputSet):   # <---- TODO: need to give more here?
        band = "N"
        detector = "GEO"

        # Define input classes ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        class RawInput(RawInput):
            """
            Raw image N_LSS_RSRF_RAW
            """
            _tags: re.Pattern = re.compile(r"N_LSS_RSRF_RAW")
            _title: str = "N LSS rsrf raw"
            _description: str = "Raw LSS flats taken with black-body calibration lamp."

        class NRsrfWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"N_WCU_OFF_RAW")
            _title: str = "N LSS WCU off"
            _description: str = "Raw data for dark subtraction in other recipes."

        class MasterDarkInput(MasterDarkInput):
            """
            Master dark MASTER_DARK_GEO
            """
            _tags: re.Pattern = re.compile(r"MASTER_DARK_GEO")

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            """
            Bad pixel BADPIX_MAP_GEO
            """
            _tags: re.Pattern = re.compile(r"BADPIX_MAP_GEO")

        class GainMapInput(GainMapInput):
            """
            Gain map
            """
            _tags: re.Pattern = re.compile(r"GAIN_MAP_GEO")

        class LinearityInput(LinearityInput):
            """
            Linearity
            """
            _tags: re.Pattern = re.compile(r"LINEARITY_GEO")

    # TODO: Check persistence

    # # ++++++++++++++++++ Intermediate products ++++++++++++++++++
    class ProductMedianNLssRsrfImg(PipelineImageProduct):
        """
        Median RSRF (QC)
        """
        _tag: str = r"MEDIAN_N_LSS_RSRF_IMG"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Median RSRF pixel map"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # # SKEL: copy product keywords from header
        # def add_properties(self):
        #     super().add_properties()
        #     self.properties.append(self.header)

    class ProductMeanNLssRsrfImg(PipelineImageProduct):
        """
        Mean RSRF (QC)
        """
        _tag: str = r"MEAN_N_LSS_RSRF_IMG"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Mean RSRF pixel map"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


    # ++++++++++++++++++ Final products ++++++++++++++++++
    class ProductMasterNLssRsrf(PipelineImageProduct):
        """
        Final Master RSRF
        """
        _tag: str = r"MASTER_N_LSS_RSRF"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Master 2D RSRF"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)


# =========================================================================================
#    Methods
# =========================================================================================

#   Method for processing
    def process_images(self) -> [PipelineImageProduct]:
        """do something more fancy in the future"""
        # Load raw image
        spec_flat_hdr = \
            cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)

        # Final RSRF
        combined_master_hdr = cpl.core.PropertyList()
        combined_master_img = self.combine_images(raw_images, "median")

        # Mean combine
        combined_mean_hdr = cpl.core.PropertyList()
        combined_mean_img = self.combine_images(raw_images, "average")

        # Median combine
        combined_median_hdr = cpl.core.PropertyList()
        combined_median_img = self.combine_images(raw_images, "median")
        return [
            self.ProductMasterNLssRsrf(self, combined_master_hdr, combined_master_img),
            self.ProductMeanNLssRsrfImg(self, combined_mean_hdr, combined_mean_img),
            self.ProductMedianNLssRsrfImg(self, combined_median_hdr, combined_median_img),
        ]

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
class MetisNLssRsrf(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_rsrf"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Create spectroscopic relative spectral response function (RSRF) for the GEO detector"
    _description: str = """\
    Create relative spectral response function for the GEO LSS detector

    Inputs
        N_LSS_RSRF_RAW: Raw RSRF images [1-n]
        N_WCU_OFF_RAW:  raw WCU OFF background frames [1-n]
        MASTER_DARK_GEO: Master dark frame [optional?]
        BADPIX_MAP_GEO:  Bad-pixel map for GEO detector [optional]
        PERSISTENCE_MAP: Persistence map [optional]
        GAIN_MAP_GEO:    Gain map for GEO detector
        LINEARITY_GEO:   Linearity map for GEO detector

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        MASTER_N_LSS_RSRF:     Master flat (RSRF) frame
        MEDIAN_N_LSS_RSRF_IMG: Median map (QC)
        MEAN_N_LSS_RSRF_IMG:   Mean map (QC)
    """
# TODO: Check whether WCU_OFF frames are necessary as input (cf. ifu rsrf recipe)

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy algorithm description follows ***TBD***""" # TODO: Write description

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
# Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO: Implement real parameters
    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("average", "median"),
        ),
    ])     # no parameters defined in DRLD
# Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    implementation_class = MetisNLssRsrfImpl

