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

from pymetis.engine.core.parameter import ParameterList
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import SinglePipelineInput
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_table

from pymetis.instruments.metis.dataitems.img.basicreduced import SciCalibrated
from pymetis.instruments.metis.dataitems.hci.hci import (AdiCalibrated, SciCentred, CentroidTab, SciSpeckle,
                                                         SciHifilt, SciDerotatedPsfsub, SciDerotated,
                                                         SciContrastRadprof, SciContrastAdi, SciThroughput,
                                                         SciCoverage, SciSnr, PsfMedian)
from pymetis.instruments.metis.inputs import RawInput
from pymetis.instruments.metis.qc.hci import (HciSciNExp, HciSciSnrMean, HciSciSnrPeak,
                                              HciSciContrastRawLamd, HciSciContrastAdiLamd, HciSciFwhm)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor


class MetisImgAdiCgrphImpl(RawImageProcessor, MetisRecipeImpl):
    """
    One recipe for both bands and the RAVC / CVC coronagraphs (DRLD `metis_img_adi_cgrph`).

    The recipe carries no band or coronagraph of its own: the band comes from the tag of
    the calibrated science frames (`LM_SCI_CALIBRATED` / `N_SCI_CALIBRATED`) and the
    coronagraph from the throughput curve (`{band}_{cgrph}_SCI_THROUGHPUT`), the one input
    on the DRLD card whose tag names it. Products and QC parameters are declared as
    templates and resolved per run from those tags; a set of frames that does not
    determine the coronagraph is refused, never guessed from a header.
    """

    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = SciCalibrated

        class ThroughputInput(SinglePipelineInput):
            Item = SciThroughput

        raw: RawInput
        throughput: ThroughputInput

    class ProductSet(PipelineProductSet):
        SciCalibrated = AdiCalibrated
        SciCentred = SciCentred
        CentroidTab = CentroidTab
        SciSpeckle = SciSpeckle
        SciHifilt = SciHifilt
        SciDerotatedPsfsub = SciDerotatedPsfsub
        SciDerotated = SciDerotated
        SciContrastRadprof = SciContrastRadprof
        SciContrastAdi = SciContrastAdi
        SciThroughput = SciThroughput
        SciCoverage = SciCoverage
        SciSnr = SciSnr
        PsfMedian = PsfMedian

    class Qc(QcParameterSet):
        SciNExp = HciSciNExp
        SciSnrMean = HciSciSnrMean
        SciSnrPeak = HciSciSnrPeak
        SciContrastRawLamd = HciSciContrastRawLamd
        SciContrastAdiLamd = HciSciContrastAdiLamd
        SciFwhm = HciSciFwhm

    def process(self) -> set[DataItem]:
        image = self.inputset.raw.load_data('DET1.DATA')[0]
        table = create_dummy_table()
        primary_header = create_dummy_header()

        images = ('SciCalibrated', 'SciCentred', 'SciSpeckle', 'SciHifilt', 'SciDerotatedPsfsub',
                  'SciDerotated', 'SciCoverage', 'SciSnr', 'PsfMedian')
        tables = ('CentroidTab', 'SciContrastRadprof', 'SciContrastAdi', 'SciThroughput')

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.SciContrastAdiLamd(None),
            self.Qc.SciContrastRawLamd(None),
            self.Qc.SciFwhm(None),
            self.Qc.SciNExp(None),
            self.Qc.SciSnrMean(None),
            self.Qc.SciSnrPeak(None),
        ))

        return {
            getattr(self.ProductSet, name)(primary_header, Hdu(create_dummy_header(), data, name='DET1.DATA'))
            for names, data in ((images, image), (tables, table))
            for name in names
        }


class MetisImgAdiCgrph(Recipe):
    _name: str = "metis_img_adi_cgrph"
    _version: str = "0.1"
    _author: str = "Jennifer Karr, A*"
    _email: str = "jkarr@asiaa.sinica.edu.tw"
    _synopsis: str = "ADI post-processing for the RAVC and CVC coronagraphs in the LM and N bands"

    _matched_keywords: frozenset[str] = frozenset({'DRS.FILTER', 'DRS.MASK'})
    _algorithm = """TODO"""

    parameters = ParameterList([])

    Impl = MetisImgAdiCgrphImpl
