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

from pymetis.dataitems import DataItem
from pymetis.dataitems.adc.adc import AdcSlitloss
from pymetis.dataitems.lss.curve import LssDistSol, LssWaveGuess
from pymetis.dataitems.lss.raw import LssSciRaw
from pymetis.dataitems.lss.response import MasterResponse, StdTransmission
from pymetis.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.dataitems.lss.science import LssObjMap, LssSkyMap, LssSci1d, LssSci2d, LssSciFlux1d, \
    LssSciFlux2d, LssSciFluxTellCorr1d
from pymetis.dataitems.lss.std import AoPsfModel
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

        class MasterLssDistSolInput(SinglePipelineInput):
            Item = LssDistSol

        class MasterLssWaveGuessInput(SinglePipelineInput):
            Item = LssWaveGuess

        class MasterLssResponseInput(SinglePipelineInput):
            Item = MasterResponse

        class MasterStdTransmissionInput(SinglePipelineInput):
            Item = StdTransmission

        class MasterAdcSlitlossInput(SinglePipelineInput):
            Item = AdcSlitloss

        class MasterAoPsfModel(SinglePipelineInput):
            Item = AoPsfModel


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


    ProductLssSciObjMap = LssObjMap
    ProductLssSciSkyMap = LssSkyMap
    ProductLssSci1d = LssSci1d
    ProductLssSci2d = LssSci2d
    ProductLssSciFlux1d = LssSciFlux1d
    ProductLssSciFlux2d = LssSciFlux2d
    ProductLssSciFluxTellCorr1d = LssSciFluxTellCorr1d

    # CAVEAT: Dummy routine only! Will be replaced with functionality -------
    # Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process(self) -> set[DataItem]:
        """do something more fancy in the future"""
        # Load raw image
        sci_raw_hdr = cpl.core.PropertyList()
        sci_raw_images = self.inputset.load_raw_images()

        """Create dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        image = self._create_dummy_image()
        table = self._create_dummy_table()

        # Write files
        return {
            self.ProductLssSci1d(header, table),
            self.ProductLssSci2d(header, image),
            self.ProductLssSciFlux1d(header, table),
            self.ProductLssSciFluxTellCorr1d(header, table),
            self.ProductLssSciFlux2d(header, image),
            self.ProductLssSciObjMap(header, image),
            self.ProductLssSciSkyMap(header, image),
        }
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++
