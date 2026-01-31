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
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.dataitems.adc.adc import AdcSlitloss
from pymetis.dataitems.lss.curve import LssDistSol, LssWaveGuess
from pymetis.dataitems.lss.raw import LssRaw
from pymetis.dataitems.lss.response import MasterResponse, StdTransmission
from pymetis.dataitems.lss.rsrf import MasterLssRsrf
from pymetis.dataitems.lss.science import LssObjMap, LssSkyMap, LssSci1d, LssSci2d, LssSciFlux1d, \
    LssSciFlux2d, LssSciFluxTellCorr1d
from pymetis.dataitems.lss.std import AoPsfModel
from pymetis.classes.inputs import RawInput, \
    SinglePipelineInput, AtmLineCatInput, OptionalInputMixin, PersistenceMapInput, GainMapInput, \
    LinearityInput, BadPixMapInput
from pymetis.classes.prefab import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table


class MetisLssSciImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssRaw

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass

        class BadPixMapInput(BadPixMapInput):
            pass

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

    class ProductSet(PipelineProductSet):
        LssSciObjMap = LssObjMap
        LssSciSkyMap = LssSkyMap
        LssSci2d = LssSci2d
        LssSci1d = LssSci1d
        LssSciFlux2d = LssSciFlux2d
        LssSciFlux1d = LssSciFlux1d
        LssSciFluxTellCorr1d = LssSciFluxTellCorr1d

    # CAVEAT: Dummy routine only! Will be replaced with functionality -------
    # Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process(self) -> set[DataItem]:
        """do something more fancy in the future"""
        # Load raw image
        sci_raw_hdr = cpl.core.PropertyList()
        sci_raw_images = self.inputset.raw.load_data('DET1.DATA')

        self.inputset.raw.use()

        """Create dummy file (should do something more fancy in the future)"""
        primary_header = create_dummy_header()
        image = create_dummy_image()
        table = create_dummy_table()

        header_lss_sci_1d = create_dummy_header()
        header_lss_sci_2d = create_dummy_header()
        header_lss_sci_sky_map = create_dummy_header()
        header_lss_sci_obj_map = create_dummy_header()
        header_lss_sci_flux_1d = create_dummy_header()
        header_lss_sci_flux_2d = create_dummy_header()
        header_lss_sci_flux_tell_corr1d = create_dummy_header()

        # Write files
        return {
            self.ProductLssSci1d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_1d, table, name='TABLE')
            ),
            self.ProductLssSci2d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_2d, image, name='IMAGE')
            ),
            self.ProductLssSciFlux1d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_flux_1d, table, name='TABLE')
            ),
            self.ProductLssSciFlux2d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_flux_2d, image, name='IMAGE')
            ),
            self.ProductLssSciFluxTellCorr1d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_flux_tell_corr1d, table, name='TABLE')
            ),
            self.ProductLssSciObjMap(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_obj_map, image, name='IMAGE')
            ),
            self.ProductLssSciSkyMap(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_sky_map, image, name='IMAGE'),
            ),
        }
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++
