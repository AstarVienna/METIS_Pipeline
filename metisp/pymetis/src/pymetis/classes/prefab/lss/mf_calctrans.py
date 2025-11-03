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

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.molecfit.model import MfBestFitTable
from pymetis.dataitems.synth import LssSynthTrans
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.inputs.mixins import LsfKernelInputSetMixin, AtmLineCatInputSetMixin, AtmProfileInputSetMixin
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.utils.dummy import create_dummy_table, create_dummy_header


class MetisLssMfCalctransImpl(MetisRecipeImpl):
    class InputSet(AtmLineCatInputSetMixin, AtmProfileInputSetMixin, LsfKernelInputSetMixin, PipelineInputSet):
        class MfBestFitTableInput(SinglePipelineInput):
            Item = MfBestFitTable

    # TODO: Check whether calctrans creates the transmission file directly, so it should not be defined here
    ProductTransmission = LssSynthTrans

    # =========================================================================================
    #    Methods
    # =========================================================================================

    #   Method for processing
    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""

        # TODO: Invoke mf_calctrans here

        # TODO: Check whether calctrans creates the Transmission file - if so, no need to
        # write it out here again
        best_fit_table = self.inputset.mf_best_fit_table.load_data('TABLE')

        primary_header = self.inputset.mf_best_fit_table.item.primary_header
        header_transmission = create_dummy_header()
        table = create_dummy_table(8)

        return {
            self.ProductTransmission(
                primary_header,
                Hdu(header_transmission, table, name='TABLE'),
            )
        }

