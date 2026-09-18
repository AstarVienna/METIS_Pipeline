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

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import SinglePipelineInput
from pymetis.engine.qc import QcParameterSet, QcParameter
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_image, create_dummy_table

from pymetis.instruments.metis.inputs import (RawInput, OptionalInputMixin, PersistenceMapInput,
                                              GainMapInput, LinearityInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import DarkImageProcessor
from pymetis.instruments.metis import qc
from pymetis.instruments.metis import dataitems


class MetisLssSciImpl(DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = dataitems.LssRaw

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class MasterRsrfInput(SinglePipelineInput):
            Item = dataitems.MasterLssRsrf

        class MasterLssDistSolInput(SinglePipelineInput):
            Item = dataitems.LssDistSol

        class MasterLssWaveGuessInput(SinglePipelineInput):
            Item = dataitems.LssWaveGuess

        class MasterLssResponseInput(SinglePipelineInput):
            Item = dataitems.MasterResponse

        class MasterStdTransmissionInput(OptionalInputMixin, SinglePipelineInput):
            Item = dataitems.StdTransmission

        class MasterAdcSlitlossInput(SinglePipelineInput):
            Item = dataitems.AdcSlitloss

        class MasterAoPsfModel(SinglePipelineInput):
            Item = dataitems.AoPsfModel

        raw: RawInput
        persistence_map: PersistenceMapInput
        gain_map: GainMapInput
        linearity: LinearityInput
        master_rsrf: MasterRsrfInput
        master_lss_dist_sol: MasterLssDistSolInput
        master_lss_wave_guess: MasterLssWaveGuessInput
        master_lss_response: MasterLssResponseInput
        master_std_transmission: MasterStdTransmissionInput
        master_adc_slitloss: MasterAdcSlitlossInput
        master_ao_psf_model: MasterAoPsfModel

    class ProductSet(PipelineProductSet):
        LssSciObjMap = dataitems.LssObjMap
        LssSciSkyMap = dataitems.LssSkyMap
        LssSci2d = dataitems.LssSci2d
        LssSci1d = dataitems.LssSci1d
        LssSciFlux2d = dataitems.LssSciFlux2d
        LssSciFlux1d = dataitems.LssSciFlux1d

    class Qc(QcParameterSet):
        class FluxSnr(QcParameter):
            _name_template = "QC {band} LSS SCI FLUX SNR"
            _type = float
            _unit = None
            _description_template = "Signal-to-noise ratio of flux calibrated science spectrum"
            _comment = None

        class FluxNoiseLevel(QcParameter):
            _name_template = "QC {band} LSS SCI FLUX NOISELEV"
            _type = float
            _unit = "Jansky"
            _default = None
            _description_template = "Noise level of flux calibrated science spectrum"
            _comment = None

        Snr = qc.lss.LssSnr
        NoiseLevel = qc.lss.LssNoiseLevel
        InterorderLevel = qc.lss.LssInterorderLevel
        WaveCalDevMean = qc.lss.LssWaveCalDevMean
        WaveCalFwhm = qc.lss.LssWaveCalFwhm
        WaveCalNIdent = qc.lss.LssWaveCalNIdent
        WaveCalNMatch = qc.lss.LssWaveCalNMatch
        WaveCalPolyDeg = qc.lss.LssWaveCalPolyDeg
        WaveCalPolyCoeffN = qc.lss.LssWaveCalPolyCoeffN
    # CAVEAT: Dummy routine only! Will be replaced with functionality -------
    # Dummy routine start +++++++++++++++++++++++++++++++++++++++++++++++++++
    def process(self) -> set[DataItem]:
        """do something more fancy in the future"""
        # Load raw image
        _sci_raw_hdr = cpl.core.PropertyList()
        _sci_raw_images = self.inputset.raw.load_data('DET1.DATA')

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
        _header_lss_sci_flux_tell_corr1d = create_dummy_header()

        # Write files
        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.FluxNoiseLevel(None),
            self.Qc.FluxSnr(None),
            self.Qc.InterorderLevel(None),
            self.Qc.NoiseLevel(None),
            self.Qc.Snr(None),
            self.Qc.WaveCalDevMean(None),
            self.Qc.WaveCalFwhm(None),
            self.Qc.WaveCalNIdent(None),
            self.Qc.WaveCalNMatch(None),
            self.Qc.WaveCalPolyCoeffN(None),
            self.Qc.WaveCalPolyDeg(None),
        ))

        return {
            self.ProductSet.LssSci1d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_1d, table, name='TABLE')
            ),
            self.ProductSet.LssSci2d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_2d, image, name='IMAGE')
            ),
            self.ProductSet.LssSciFlux1d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_flux_1d, table, name='TABLE')
            ),
            self.ProductSet.LssSciFlux2d(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_flux_2d, image, name='IMAGE')
            ),
            #self.ProductSet.LssSciFluxTell1d(
            #    copy.deepcopy(primary_header),
            #    Hdu(header_lss_sci_flux_tell_corr1d, table, name='TABLE')
            #),
            self.ProductSet.LssSciObjMap(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_obj_map, image, name='IMAGE')
            ),
            self.ProductSet.LssSciSkyMap(
                copy.deepcopy(primary_header),
                Hdu(header_lss_sci_sky_map, image, name='IMAGE'),
            ),
        }
# Dummy routine end +++++++++++++++++++++++++++++++++++++++++++++++++++
