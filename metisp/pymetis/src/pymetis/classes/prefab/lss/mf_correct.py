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
from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.lss.science import LssSciFlux1d, LssSciFluxTellCorr1d
from pymetis.dataitems.synth import LssSynthTrans
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.utils.dummy import create_dummy_header, create_dummy_table


class MetisLssMfCorrectImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class LssSciFlux1dInput(SinglePipelineInput):
            Item = LssSciFlux1d

        class TransmissionInput(SinglePipelineInput):
            Item = LssSynthTrans

    ProductTellCorrFinal = LssSciFluxTellCorr1d

    def mf_correct(self):
        pass

    def process(self) -> set[DataItem]:
        """Create dummy file (should do something more fancy in the future)"""

        lss_sci_flux = self.inputset.lss_sci_flux1d.load_data()
        self.mf_correct()

        # TODO: Check whether calctrans creates the Transmission file - if so, no need to
        # write it out here again
        header = lss_sci_flux.header
        table = create_dummy_table()

        return {
            self.ProductTellCorrFinal(header, table),
        }

