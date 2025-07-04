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

from pymetis.classes.mixins import BandNMixin
from pymetis.classes.prefab.lss.std import MetisLssStdImpl
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor

from pymetis.classes.inputs import (SinglePipelineInput, BadPixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin, FluxstdCatalogInput, AtmLineCatInput,
                                    PersistenceMapInput)


class MetisNLssStdImpl(MetisLssStdImpl):
    class InputSet(BandNMixin, RawImageProcessor.InputSet):
        pass

        # RAW FILES ++++++++++++++++++++++++++++++++++++++++++++++++
        class RawInput(RawInput):
            """
            Raw standard star observations
            """
            _tags: re.Pattern = re.compile(r"N_LSS_STD_RAW")
            _title: str = "N LSS std raw"
            _description: str = "Raw spectra of standard stars."

        # MASTER CALIBS ++++++++++++++++++++++++++++++++++++++++++++
        class MasterPersistenceMap(PersistenceMapInput):
            """
            Persistence map
            """
            _tags: re.Pattern = re.compile(r"PERSISTENCE_MAP")

        class MasterDarkInput(MasterDarkInput):
            """
            Master dark MASTER_DARK_GEO
            """
            _tags: re.Pattern = re.compile(r"MASTER_DARK_GEO")

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
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

        class MasterRsrfInput(SinglePipelineInput):
            """
            MASTER N LSS RSRF
            """
            _tags: re.Pattern = re.compile(r"MASTER_N_LSS_RSRF")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "MASTER_RSRF"
            _description: str = "Master 2D RSRF"

        class MasterNLssDistSol(SinglePipelineInput):
            """
            Distortion solution
            """
            _tags: re.Pattern = re.compile(r"N_LSS_TRACE")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Trace location"
            _description: str = "Table with polynomials describing the location of the traces on the detector"

        class MasterNLssDistSol(SinglePipelineInput):
            """
            Distortion solution
            """
            _tags: re.Pattern = re.compile(r"N_LSS_DIST_SOL")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Distortion solution"
            _description: str = "Distortion solution for rectifying"

        class MasterNLssWaveGuess(SinglePipelineInput):
            """
            Wavelength solution first guess
            """
            _tags: re.Pattern = re.compile(r"N_LSS_WAVE_GUESS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "First guess of the wavelength solution"
            _description: str = "First guess of the wavelength solution"

        class MasterAoPsfModel(SinglePipelineInput):
            """
            AO PSF MODEL
            """
            _tags: re.Pattern = re.compile(r"AO_PSF_MODEL")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "AO induced PSF model"
            _description: str = "Model of the PSF induced by the AO"

        # STATIC CALIBS ++++++++++++++++++++++++++++++++++++++++++++
        class MasterNAdcSlitloss(SinglePipelineInput):
            """
            Slitloss file
            """
            _tags: re.Pattern = re.compile(r"N_ADC_SLITLOSS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Slitloss file"
            _description: str = "Slitlosses induced by the ADC"

        class MasterNLssSynthTrans(SinglePipelineInput):
            """
            Synthetic Transmission
            """
            _tags: re.Pattern = re.compile(r"N_LSS_SYNTH_TRANS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Transmission model"
            _description: str = "Transmission of the Earth's atmosphere for a quick telluric correction"

        class MasterAtmLineCat(AtmLineCatInput):
            """
            Catalogue of atmospheric lines
            """
            _tags: re.Pattern = re.compile(r"ATM_LINE_CAT")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Line catalogue of atmospheric lines"
            _description: str = "Catalogue containing a line list of atmospheric molecular lines"

        class MasterRefStdCat(FluxstdCatalogInput):
            """
            Catalogue of reference stars
            """
            _tags: re.Pattern = re.compile(r"REF_STD_CAT")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Reference catalogue of standard stars"
            _description: str = "Catalogue with spectra of standard reference stars"


    # ++++++++++++ Intermediate / QC products ++++++++++++

    class ProductNLssStdObjMap(PipelineImageProduct):
        """
        Pixel map of object pixels
        """
        _tag: str = r"N_LSS_STD_OBJ_MAP"
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

    class ProductNLssStdSkyMap(PipelineImageProduct):
        """
        Pixel map of sky pixels
        """
        _tag: str = r"N_LSS_STD_SKY_MAP"
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
    class ProductMasterNResponse(PipelineImageProduct):
        """
        Final Master Response Curve
        """
        _tag: str = r"MASTER_N_RESPONSE"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Master response curve for flux calibration"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    class ProductStdTransmission(PipelineImageProduct):
        """
        Final Transmission
        """
        _tag: str = r"STD_TRANSMISSION"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Transmission function derived from standard star"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    class ProductNLssWave(PipelineImageProduct):
        """
        Final Wave solution
        """
        _tag: str = r"N_LSS_STD_WAVE"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Wavelength solution derived from standard star"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    class ProductNLssStd1d(PipelineImageProduct):
        """
        Final 1D spectrum of standard star
        """
        _tag: str = r"N_LSS_STD_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D spectrum of standard star"
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


#   Method for loading images
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

#   Method for data processing

# CAVEAT: Dummy routine only! Will be replaced with functionality -------
# Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process(self) -> set[DataItem]:
        # Load raw image
        std_raw_hdr = \
            cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)

        """Create dummy file (should do something more fancy in the future)"""
        # header = self._create_dummy_header()
        # PipelineImageProducts
        ProductNLssStdObjMapHdr = self._create_dummy_header()
        ProductNLssStdSkyMapHdr = self._create_dummy_header()
        image = self._create_dummy_image()

        # PipelineTableProducts
        ProductMasterNResponseHdr = self._create_dummy_header()
        ProductStdTransmissionHdr = self._create_dummy_header()
        ProductNLssStd1dHdr = self._create_dummy_header()
        table = self._create_dummy_table()

        # Write files
        return [
            self.ProductMasterNResponse(self, ProductMasterNResponseHdr, image),
            self.ProductStdTransmission(self, ProductStdTransmissionHdr, image),
            self.ProductNLssStd1d(self, ProductNLssStd1dHdr, image),
            self.ProductNLssStdObjMap(self, ProductNLssStdObjMapHdr, image),
            self.ProductNLssStdSkyMap(self, ProductNLssStdSkyMapHdr, image),
        ]
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++


# =========================================================================================
#    MAIN PART
# =========================================================================================


# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe

class MetisNLssStd(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_std"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Reduction of the standard star frames for determining the response function (flux calibration) and/or the transmission (telluric correction)"
    _description: str = """\
    Reduction of the standard star frames for determining the response function (flux calibration) and/or the transmission (telluric correction)

    Inputs
        N_LSS_STD_RAW:     Raw standard star images [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        LINEARITY_GEO:      Linearity map for GEO detector
        GAIN_MAP_GEO:       Gain map for GEO detector
        BADPIX_MAP_GEO:     Bad-pixel map for GEO detector [optional]
        MASTER_DARK_GEO:    Master dark frame [optional?]
        MASTER_N_LSS_RSRF: Master flat (RSRF) frame
        N_LSS_DIST_SOL:    Distortion solution
        N_LSS_WAVE_GUESS:  First guess of the wavelength solution
        AO_PSF_MODEL:       Model of the AO PSF
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        N_ADC_SLITLOSS:    Slitloss information
        N_SYNTH_TRANS:     Synthetic model of the Earth's atmopshere transmission
        REF_STD_CAT:        Catalogue of standard stars

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        N_LSS_STD_OBJ_MAP: Pixel map of the object pixels (QC)
        N_LSS_STD_SKY_MAP: Pixel map of the sky pixels (QC)
        N_LSS_STD_1D:      Coadded, wavelength calibrated, collapsed 1D spectrum of the standard star
        N_LSS_STD_WAVE:    Wavelength solution based on std star
        STD_TRANSMISSION:   Transmission of the Earth's atmosphere derived from the STD for telluric correction  [optional]
        MASTER_N_RESPONSE: Response function for flux calibration
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
    implementation_class = MetisNLssStdImpl
