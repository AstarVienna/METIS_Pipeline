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
from pymetis.classes.dataitems.lss.curve import LssDistSol
from pymetis.classes.dataitems.lss.raw import LssStdRaw
from pymetis.classes.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.classes.dataitems.lss.trace import LssTrace
from pymetis.classes.dataitems.synth import LssSynthTrans
from pymetis.classes.inputs import RawInput, PersistenceInputSetMixin, MasterDarkInput, BadPixMapInputSetMixin, \
    GainMapInputSetMixin, SinglePipelineInput
from pymetis.classes.inputs.mixins import LsfKernelInputSetMixin, AtmLineCatInputSetMixin
from pymetis.classes.prefab import DarkImageProcessor


class MetisLssStdImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, LsfKernelInputSetMixin,
                   AtmLineCatInputSetMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssStdRaw

        MasterDarkInput = MasterDarkInput

        class MasterRsrfInput(SinglePipelineInput):
            Item = MasterLssRsrf

        class MasterLssTrace(SinglePipelineInput):
            Item = LssTrace

        class MasterLssDistSol(SinglePipelineInput):
            Item = LssDistSol

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

        class LssSynthTransInput(SinglePipelineInput):
            Item = LssSynthTrans

        class MasterRefStdCat(FluxstdCatalogInput):
            """
            Catalogue of reference stars
            """
            _tags: re.Pattern = re.compile(r"REF_STD_CAT")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Reference catalogue of standard stars"
            _description: str = "Catalogue with spectra of standard reference stars"


    # ++++++++++++ Intermediate / QC products ++++++++++++

    ProductLssStdObjMap = LssStdObjMap

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
