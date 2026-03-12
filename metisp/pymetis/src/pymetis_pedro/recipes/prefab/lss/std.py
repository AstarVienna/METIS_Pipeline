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
import copy

import cpl

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.adc.adc import AdcSlitloss
from pymetis.dataitems.lss.curve import LssDistSol, LssWaveGuess
from pymetis.dataitems.lss.raw import LssStdRaw
from pymetis.dataitems.lss.response import MasterResponse, StdTransmission
from pymetis.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.dataitems.lss.science import LssSkyMap, LssObjMap
from pymetis.dataitems.lss.std import RefStdCat, AoPsfModel, LssStd1d
from pymetis.dataitems.lss.trace import LssTrace
from pymetis.dataitems.synth import SynthTrans
from pymetis.classes.inputs import RawInput, PersistenceInputSetMixin, MasterDarkInput, BadPixMapInputSetMixin, \
    GainMapInputSetMixin, SinglePipelineInput, FluxstdCatalogInput, MasterRsrfInput, LinearityInputSetMixin
from pymetis.classes.inputs.mixins import AtmLineCatInputSetMixin
from pymetis.recipes.prefab import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table


class MetisLssStdImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, BadPixMapInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin,
                   AtmLineCatInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssStdRaw

        MasterDarkInput = MasterDarkInput
        MasterRsrfInput = MasterRsrfInput

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
            Item = SynthTrans

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
        raw_images = self.inputset.raw.load_data('DET1.DATA')
        primary_header = self.inputset.raw.items[0].primary_header

        """Create dummy file (should do something more fancy in the future)"""
        # header = create_dummy_header()
        # PipelineImageProducts
        product_lss_std_obj_map_hdr = create_dummy_header()
        product_lss_std_sky_map_hdr = create_dummy_header()
        image = create_dummy_image()

        # PipelineTableProducts
        product_master_response_hdr = create_dummy_header()
        product_std_transmission_hdr = create_dummy_header()
        product_lss_std1d_hdr = create_dummy_header()
        table = create_dummy_table()

        # Write files
        return {
            self.ProductMasterResponse(
                copy.deepcopy(primary_header),
                Hdu(product_master_response_hdr, table, name='TABLE')
            ),
            self.ProductStdTransmission(
                copy.deepcopy(primary_header),
                Hdu(product_std_transmission_hdr, table, name='TABLE')
            ),
            self.ProductLssStd1d(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std1d_hdr, table, name='TABLE')
            ),
            self.ProductLssStdObjMap(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std_obj_map_hdr, image, name='IMAGE')
            ),
            self.ProductLssStdSkyMap(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std_sky_map_hdr, image, name='IMAGE')
            ),
        }
