"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

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

import cpl
from cpl.core import Msg

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.adc.adc import AdcSlitloss
from pymetis.classes.dataitems.common import AtmLineCatalog
from pymetis.classes.dataitems.lss.lss import LssSciRaw
from pymetis.classes.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.classes.inputs import RawInput, PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, \
    LinearityInputSetMixin, SinglePipelineInput
from pymetis.classes.inputs.mixins import AtmLineCatInputSetMixin
from pymetis.classes.prefab import DarkImageProcessor


class MetisLssSciImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin,
                   AtmLineCatInputSetMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssSciRaw

        class MasterRsrfInput(SinglePipelineInput):
            Item = MasterLssRsrf

        class MasterLmLssDistSol(SinglePipelineInput):
            Item = LssDistSol
            """
            Distortion solution
            """
            _tags: re.Pattern = re.compile(r"LM_LSS_DIST_SOL")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Distortion solution"
            _description: str = "Distortion solution for rectifying"

        class MasterLmLssWaveGuess(SinglePipelineInput):
            Item = LssWaveGuess
            """
            Wavelength solution first guess
            """
            _tags: re.Pattern = re.compile(r"LM_LSS_WAVE_GUESS")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "First guess of the wavelength solution"
            _description: str = "First guess of the wavelength solution"

        class MasterLmLssResponse(SinglePipelineInput):
            Ite = MasterResponse
            """
            Master response
            """
            _tags: re.Pattern = re.compile(r"MASTER_LM_RESPONSE")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Response function"
            _description: str = "Master response curve for flux calibration"

        class MasterStdTransmission(SinglePipelineInput):
            Item = StdTransmission
            """
            Transmission from the standard star (optional)
            """
            _tags: re.Pattern = re.compile(r"STD_TRANSMISSION")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Standard transmission"
            _description: str = "Transmission function derived from standard star"

        # --------------------------------------------------------------------
        # TODO:
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
        class MasterLmAdcSlitloss(SinglePipelineInput):
            Item = AdcSlitloss

    ProductLssSciObjMap = LssSciObjMap
    ProductLssSciSkyMap = LssSciSkyMap
    ProductLssSci1d = LssSci1d
    ProductLssSci2d = LssSci2d
    ProductLssSciFlux1d = LssSciFlux1d
    ProductLssSciFlux2d = LssSciFlux2d


    # ++++++++++++ Intermediate / QC products ++++++++++++
    class ProductLmLssSciObjMap(PipelineImageProduct):
        """
        Pixel map of object pixels
        """
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

    class ProductLmLssSciSkyMap(PipelineImageProduct):
        """
        Pixel map of sky pixels
        """
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
    class ProductLmLssSci1d(PipelineTableProduct):
        """
        Final 1D spectrum of standard star
        """
        _tag: str = r"LM_LSS_SCI_1D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "1D spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


    class ProductLmLssSci2d(PipelineImageProduct):
        """
        Final 2D spectrum of standard star
        """
        _tag: str = r"LM_LSS_SCI_2D"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "2D spectrum of science target"
        _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


    class ProductLmLssSciFlux1d(PipelineTableProduct):
        """
        Final flux calibrated 1D spectrum of standard star
        """
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


    class ProductLmLssSciFlux2d(PipelineImageProduct):
        """
        Final flux calibrated 2D spectrum of standard star
        """
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

    # TODO: What about the 2d version?
    class ProductLmLssSciFluxTell1d(PipelineTableProduct):
        """
        Final flux calibrated, telluric corrected 1D spectrum of standard star
        """
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
            self.ProductLmLssSci1d(header, table),
            self.ProductLmLssSci2d(header, image),
            self.ProductLmLssSciFlux1d(header, table),
            self.ProductLmLssSciFluxTell1d(header, table),
            self.ProductLmLssSciFlux2d(header, image),
            self.ProductLmLssSciObjMap(header, image),
            self.ProductLmLssSciSkyMap(header, image),
        }
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++
