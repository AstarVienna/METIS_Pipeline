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

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.adc.adc import AdcSlitloss
from pymetis.classes.dataitems.lss.curve import LssDistSol, LssWaveGuess
from pymetis.classes.dataitems.lss.raw import LssStdRaw
from pymetis.classes.dataitems.lss.response import MasterResponse, StdTransmission
from pymetis.classes.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.classes.dataitems.lss.science import LssSkyMap, LssObjMap
from pymetis.classes.dataitems.lss.std import RefStdCat, AoPsfModel, LssStd1d
from pymetis.classes.dataitems.lss.trace import LssTrace
from pymetis.classes.dataitems.synth import LssSynthTrans
from pymetis.classes.inputs import RawInput, PersistenceInputSetMixin, MasterDarkInput, BadPixMapInputSetMixin, \
    GainMapInputSetMixin, SinglePipelineInput, FluxstdCatalogInput
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

        class MasterLssWaveGuess(SinglePipelineInput):
            Item = LssWaveGuess

        class MasterAoPsfModel(SinglePipelineInput):
            Item = AoPsfModel

        # STATIC CALIBS ++++++++++++++++++++++++++++++++++++++++++++
        class MasterAdcSlitlossInput(SinglePipelineInput):
            Item = AdcSlitloss

        class LssSynthTransInput(SinglePipelineInput):
            Item = LssSynthTrans

        class RefStdCatInput(FluxstdCatalogInput):
            Item = RefStdCat
            """
            Catalogue of reference stars
            """

    # ++++++++++++ Intermediate / QC products ++++++++++++

    ProductLssStdObjMap = LssObjMap
    ProductLssStdSkyMap = LssSkyMap
    ProductMasterResponse = MasterResponse
    ProductStdTransmission = StdTransmission
    ProductLssWave = LssWaveGuess
    ProductLssStd1d = LssStd1d


    def process(self) -> set[DataItem]:
        # Load raw image
        std_raw_hdr = cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)

        """Create dummy file (should do something more fancy in the future)"""
        # header = self._create_dummy_header()
        # PipelineImageProducts
        ProductLmLssStdObjMapHdr = self._create_dummy_header()
        ProductLmLssStdSkyMapHdr = self._create_dummy_header()
        image = self._create_dummy_image()

        # PipelineTableProducts
        ProductMasterLmResponseHdr = self._create_dummy_header()
        ProductStdTransmissionHdr = self._create_dummy_header()
        ProductLmLssStd1dHdr = self._create_dummy_header()
        table = self._create_dummy_table()

        # Write files
        return {
            self.ProductMasterLmResponse(ProductMasterLmResponseHdr, image),
            self.ProductStdTransmission(ProductStdTransmissionHdr, image),
            self.ProductLmLssStd1d(ProductLmLssStd1dHdr, image),
            self.ProductLmLssStdObjMap(ProductLmLssStdObjMapHdr, image),
            self.ProductLmLssStdSkyMap(ProductLmLssStdSkyMapHdr, image),
        }
