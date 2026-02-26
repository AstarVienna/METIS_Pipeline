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
from pymetis.classes.qc import QcParameterSet, QcParameter
from pymetis.dataitems.adc.adc import AdcSlitloss
from pymetis.dataitems.lss.curve import LssDistSol, LssWaveGuess
from pymetis.dataitems.lss.raw import LssRaw
from pymetis.dataitems.lss.response import MasterResponse, StdTransmission
from pymetis.dataitems.lss.science import LssSkyMap, LssObjMap
from pymetis.dataitems.lss.std import RefStdCat, AoPsfModel, LssStd1d
from pymetis.dataitems.lss.trace import LssTrace
from pymetis.dataitems.synth import SynthTrans
from pymetis.classes.inputs import RawInput, MasterDarkInput, \
    SinglePipelineInput, FluxstdCatalogInput, MasterRsrfInput, PersistenceMapInput, BadPixMapInput, GainMapInput, \
    LinearityInput, AtmLineCatInput
from pymetis.classes.prefab import DarkImageProcessor
from pymetis.qc.lss import LssWaveCalPolyCoeffN, LssWaveCalPolyDeg, LssWaveCalNMatch, LssWaveCalNIdent, LssWaveCalFwhm, \
    LssWaveCalDevMean, LssInterorderLevel, LssSnr, LssNoiseLevel
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table


class MetisLssStdImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LssRaw

        class MasterDarkInput(MasterDarkInput):
            pass

        class MasterRsrfInput(MasterRsrfInput):
            pass

        class PersistenceInput(PersistenceMapInput):
            pass

        class BadPixMapInput(BadPixMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass

        class AtmLineCatInput(AtmLineCatInput):
            pass

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

    class ProductSet(PipelineProductSet):
        LssStdObjMap = LssObjMap
        LssStdSkyMap = LssSkyMap
        MasterResponse = MasterResponse
        StdTransmission = StdTransmission
        LssWave = LssWaveGuess
        LssStd1d = LssStd1d

    class Qc(QcParameterSet):
        class BackgroundMean(QcParameter):
            _name_template = "QC {band} LSS STD BACKGD MEAN"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "Mean value of background"

        class BackgroundMedian(QcParameter):
            _name_template = "QC {band} LSS STD BACKGD MEDIAN"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "Median value of background"

        class BackgroundStdev(QcParameter):
            _name_template = "QC {band} LSS STD BACKGD STDEV"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "Standard deviation value of background"
            _comment = None

        class Fwhm(QcParameter):
            _name_template = "QC {band} LSS STD FWHM"
            _type = float
            _unit = "Å"
            _default = None
            _description_template = "FWHM of flux standard spectrum"
            _comment = None

        class AverageLevel(QcParameter):
            _name_template = "QC {band} LSS STD AVGLEVEL"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "" # FixMe missing in DRLD
            _comment = None

        Snr = LssSnr
        NoiseLevel = LssNoiseLevel
        InterorderLevel = LssInterorderLevel
        WaveCalDevMean = LssWaveCalDevMean
        WaveCalFwhm = LssWaveCalFwhm
        WaveCalNIdent = LssWaveCalNIdent
        WaveCalNMatch = LssWaveCalNMatch
        WaveCalPolyDeg = LssWaveCalPolyDeg
        WaveCalPolyCoeffN = LssWaveCalPolyCoeffN

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
            self.ProductSet.MasterResponse(
                copy.deepcopy(primary_header),
                Hdu(product_master_response_hdr, table, name='TABLE')
            ),
            self.ProductSet.StdTransmission(
                copy.deepcopy(primary_header),
                Hdu(product_std_transmission_hdr, table, name='TABLE')
            ),
            self.ProductSet.LssStd1d(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std1d_hdr, table, name='TABLE')
            ),
            self.ProductSet.LssStdObjMap(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std_obj_map_hdr, image, name='IMAGE')
            ),
            self.ProductSet.LssStdSkyMap(
                copy.deepcopy(primary_header),
                Hdu(product_lss_std_sky_map_hdr, image, name='IMAGE')
            ),
        }
