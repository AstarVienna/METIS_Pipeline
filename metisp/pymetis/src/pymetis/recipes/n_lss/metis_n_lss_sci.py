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

from pymetis.classes.dataitems.lss.lss import LssRaw
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.rawimage import RawImageProcessor

from pymetis.classes.inputs import (SinglePipelineInput, BadPixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    LinearityInput, OptionalInputMixin, AtmLineCatInput,
                                    PersistenceMapInput)


# =========================================================================================
#    Define main class
# =========================================================================================
class MetisNLssSciImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        # RAW FILES ++++++++++++++++++++++++++++++++++++++++++++++++
        class RawInput(RawInput):
            Item = LssRaw

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

        class MasterNLssResponse(SinglePipelineInput):
            """
            Master response
            """
            _tags: re.Pattern = re.compile(r"MASTER_N_RESPONSE")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Response function"
            _description: str = "Master response curve for flux calibration"

        class MasterStdTransmission(SinglePipelineInput):
            """
            Transmission from the standard star (optional)
            """
            _tags: re.Pattern = re.compile(r"STD_TRANSMISSION")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Standard transmission"
            _description: str = "Transmission function derived from standard star"

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

        class MasterAtmLineCat(AtmLineCatInput):
            """
            Catalogue of atmospheric lines
            """
            _tags: re.Pattern = re.compile(r"ATM_LINE_CAT")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Line catalogue of atmospheric lines"
            _description: str = "Catalogue containing a line list of atmospheric molecular lines"



    # ++++++++++++ Intermediate / QC products ++++++++++++
    class ProductNLssSciObjMap(PipelineImageProduct):
        """
        Pixel map of object pixels
        """
        _tag: str = r"N_LSS_SCI_OBJ_MAP"
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

    class ProductNLssSciSkyMap(PipelineImageProduct):
        """
        Pixel map of sky pixels
        """
        _tag: str = r"N_LSS_SCI_SKY_MAP"
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
    class ProductNLssSci1d(PipelineTableProduct):
        """
        Final 1D spectrum of standard star
        """
        _tag: str = r"N_LSS_SCI_1D"
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

    class ProductNLssSci2d(PipelineImageProduct):
        """
        Final 2D spectrum of standard star
        """
        _tag: str = r"N_LSS_SCI_2D"
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

    class ProductNLssSciFlux1d(PipelineTableProduct):
        """
        Final flux calibrated 1D spectrum of standard star
        """
        _tag: str = r"N_LSS_SCI_FLUX_1D"
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


    class ProductNLssSciFlux2d(PipelineImageProduct):
        """
        Final flux calibrated 2D spectrum of standard star
        """
        _tag: str = r"N_LSS_SCI_FLUX_2D"
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

    # TODO: What about the 2d version?
    class ProductNLssSciFluxTell1d(PipelineTableProduct):
        """
        Final flux calibrated, telluric corrected 1D spectrum of standard star
        """
        _tag: str = r"N_LSS_SCI_FLUX_TELL_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D flux-calibrated, telluric corrected spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}

# =========================================================================================
#    Methods
# =========================================================================================

# CAVEAT: Dummy routine only! Will be replaced with functionality -------
# Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process(self) -> set[DataItem]:
        """do something more fancy in the future"""
        # Load raw image
        sci_raw_hdr = \
            cpl.core.PropertyList()
        sci_raw_images = self.load_images(self.inputset.raw.frameset)

        """Create dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        image = self._create_dummy_image()
        table = self._create_dummy_table()

        # Write files
        return {
            self.ProductNLssSci1d(header, table),
            self.ProductNLssSci2d(header, image),
            self.ProductNLssSciFlux1d(header, table),
            self.ProductNLssSciFluxTell1d(header, table),
            self.ProductNLssSciFlux2d(header, image),
            self.ProductNLssSciObjMap(header, image),
            self.ProductNLssSciSkyMap(header, image),
        }
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++

# =========================================================================================
#    MAIN PART
# =========================================================================================

# Define recipe main function as a class which inherits from
# the PyCPL class cpl.ui.PyRecipe

class MetisNLssSci(MetisRecipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_n_lss_sci"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Reduction of the LSS science star frames"
    _description: str = """\
    Reduction of the LSS science star frames

    Inputs
        N_LSS_SCI_RAW:     Raw science images [1-n]
        PERSISTENCE_MAP:    Persistence map [optional]
        LINEARITY_GEO:      Linearity map for GEO detector
        GAIN_MAP_GEO:       Gain map for GEO detector
        BADPIX_MAP_GEO:     Bad-pixel map for GEO detector [optional]
        MASTER_DARK_GEO:    Master dark frame [optional?]
        MASTER_N_LSS_RSRF: Master flat (RSRF) frame
        N_LSS_DIST_SOL:    Distortion solution
        N_LSS_WAVE_GUESS:  First guess of the wavelength solution
        ATM_LINE_CAT:       Catalogue of atmospheric lines
        N_ADC_SLITLOSS:    Slitloss information
        STD_TRANSMISSION:   Transmission of the Earth's atmosphere derived from the STD for telluric correction  [optional]
        MASTER_N_RESPONSE: Response function for flux calibration

     Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.SLIT

    Outputs
        N_LSS_SCI_OBJ_MAP: Pixel map of the object pixels (QC)
        N_LSS_SCI_SKY_MAP: Pixel map of the sky pixels (QC)
        N_LSS_SCI_2D:      Coadded, wavelength calibrated, 2D spectrum of the science object
        N_LSS_SCI_1D:      Coadded, wavelength calibrated, collapsed 1D spectrum of the science object
        N_LSS_SCI_FLUX_2D: Coadded, wavelength + flux calibrated, 2D spectrum of the science object
        N_LSS_SCI_FLUX_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object
        N_LSS_SCI_FLUX_TELL_1D: Coadded, wavelength + flux calibrated, collapsed 1D spectrum of the science object (optional)
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
    implementation_class = MetisNLssSciImpl
