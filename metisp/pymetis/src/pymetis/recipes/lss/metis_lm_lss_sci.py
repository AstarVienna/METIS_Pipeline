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

from pymetis.classes.mixins import Detector2rgMixin

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor

from pymetis.classes.recipes.impl import MetisRecipeImpl

from pymetis.classes.inputs import (RawInput, SinglePipelineInput, BadpixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin, AtmLineCatInput,
                                    PersistenceMapInput)
from pymetis.classes.products import PipelineProduct


# =========================================================================================
#    Define main class
# =========================================================================================
class MetisLmLssSciImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        band = "LM"
        detector = "2RG"

        # RAW FILES ++++++++++++++++++++++++++++++++++++++++++++++++
        """
        Raw standard star observations
        """
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_LSS_SCI_RAW")
            _title: str = "LM LSS sci raw"
            _description: str = "Raw spectra of sciecne objects."

        # MASTER CALIBS ++++++++++++++++++++++++++++++++++++++++++++
        """
        Persistence map
        """
        class MasterPersistenceMap(PersistenceMapInput):
            _tags: re.Pattern = re.compile(r"PERSISTENCE_MAP")

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

        """
        Distortion solution
        """
        class MasterLmLssDistSol(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_LSS_DIST_SOL")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Distortion solution"
            _description: str = "Distortion solution for rectifying"

        """
        Wavelength solution first guess
        """
        class MasterLmLssWaveGuess(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_LSS_WAVE_GUESS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "First guess of the wavelength solution"
            _description: str = "First guess of the wavelength solution"

        """
        Master response
        """
        class MasterLmLssResponse(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"MASTER_LM_RESPONSE")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Response function"
            _description: str = "Master response curve for flux calibration"

        """
        Transmission from the standard star (optional)
        """
        class MasterStdTransmission(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"STD_TRANSMISSION")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Standard transmission"
            _description: str = "Transmission function derived from standard star"

# --------------------------------------------------------------------
# CHECK THE AO PSF MODEL - why not included? forgotten????
        # """
        # AO PSF MODEL
        # """
        # class MasterAoPsfModel(SinglePipelineInput):
        #     _tags: re.Pattern = re.compile(r"AO_PSF_MODEL")
        #     _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
        #     _title: str = "AO induced PSF model"
        #     _description: str = "Model of the PSF induced by the AO"
# CHECK THE AO PSF MODEL - why not included? forgotten????
# --------------------------------------------------------------------


        # STATIC CALIBS ++++++++++++++++++++++++++++++++++++++++++++
        """
        Slitloss file
        """
        class MasterLmAdcSlitloss(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_ADC_SLITLOSS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Slitloss file"
            _description: str = "Slitlosses induced by the ADC"

        """
        Catalogue of atmospheric lines
        """
        class MasterAtmLineCat(AtmLineCatInput):
            _tags: re.Pattern = re.compile(r"ATM_LINE_CAT")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Line catalogue of atmospheric lines"
            _description: str = "Catalogue containing a line list of atmospheric molecular lines"



    # ++++++++++++ Intermediate / QC products ++++++++++++
    """
    Pixel map of object pixels
    """
    class ProductLmLssSciObjMap(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_OBJ_MAP"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Pixel map of object pixels (QC)"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    """
    Pixel map of sky pixels
    """
    class ProductLmLssSciSkyMap(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_SKY_MAP"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Pixel map of sky pixels (QC)"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)
    # ++++++++++++++++++ Final products ++++++++++++++++++
    """
    Final 1D spectrum of standard star
    """
    class ProductLmLssSci1d(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    """
    Final 2D spectrum of standard star
    """
    class ProductLmLssSci2d(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_2D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "2D spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    """
    Final flux calibrated 1D spectrum of standard star
    """
    class ProductLmLssSciFlux1d(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_FLUX_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D flux-calibrated spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)


    """
    Final flux calibrated 2D spectrum of standard star
    """
    class ProductLmLssSciFlux2d(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_FLUX_2D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "2D flux-calibrated spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    """
    Final flux calibrated, telluric corrected 1D spectrum of standard star
    """
    # TODO: What about the 2d version?
    class ProductLmLssSciFluxTell1d(PipelineProduct):
        _tag: str = r"LM_LSS_SCI_FLUX_TELL_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D flux-calibrated, telluric corrected spectrum of science target"
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

# CAVEAT: Dummy routine only! Will be replaced with functionality -------
# Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process_images(self) -> [PipelineProduct]:
        """do something more fancy in the future"""
        # Load raw image
        sci_raw_hdr = \
            cpl.core.PropertyList()
        sci_raw_images = self.load_images(self.inputset.raw.frameset)

        """Create dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        image = self._create_dummy_image()

        # Write files
        return [

            self.ProductLmLssSci1d(self, header, image),
            self.ProductLmLssSci2d(self, header, image),
            self.ProductLmLssSciFlux1d(self, header, image),
            self.ProductLmLssSciFluxTell1d(self, header, image),
            self.ProductLmLssSciFlux2d(self, header, image),
            self.ProductLmLssSciObjMap(self, header, image),
            self.ProductLmLssSciSkyMap(self, header, image),
        ]
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++

# =========================================================================================
#    MAIN PART
# =========================================================================================

# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe

class MetisLmLssSci(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_sci"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Reduction of the LSS science star frames"
    _undescription: str = """\
    Reduction of the LSS science star frames

    Inputs
        LM_LSS_SCI_RAW:     Raw science images [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        LINEARITY_2RG:      Linearity map for 2RG detector
        GAIN_MAP_2RG:       Gain map for 2RG detector
        BADPIX_MAP_2RG:     Bad-pixel map for 2RG detector [optional]
        MASTER_DARK_2RG:    Master dark frame [optional?]
        MASTER_LM_LSS_RSRF: Master flat (RSRF) frame
        LM_LSS_DIST_SOL:    Distortion solution
        LM_LSS_WAVE_GUESS:  First guess of the wavelength solution
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        LM_ADC_SLITLOSS:    Slitloss information
        STD_TRANSMISSION:   Transmission of the Earth's atmosphere derived from the STD for telluric correction  [optional]
        MASTER_LM_RESPONSE: Response function for flux calibration

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        LM_LSS_SCI_OBJ_MAP: Pixel map of the object pixels (QC)
        LM_LSS_SCI_SKY_MAP: Pixel map of the sky pixels (QC)
        LM_LSS_SCI_2D:      Coadded, wavelength calibrated, 2D spectrum of the science object
        LM_LSS_SCI_1D:      Coadded, wavelength calibrated, collapsed 1D spectrum of the science object
        LM_LSS_SCI_FLUX_2D: Coadded, wavelength + flux calibrated, 2D spectrum of the science object
        LM_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object
        LM_LSS_SCI_FLUX_TELL_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object (optional)
    """

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.SLIT'}
    _algorithm = """Fancy description follows"""

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
    implementation_class = MetisLmLssSciImpl
