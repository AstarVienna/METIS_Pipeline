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
from pymetis.dataitems.lss.science import LssSciFlux1d
from pymetis.dataitems.lss.std import LssStd1d
from pymetis.dataitems.molecfit.model import MfBestFitTable
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.inputs.mixins import AtmProfileInputSetMixin, AtmLineCatInputSetMixin, LsfKernelInputSetMixin
from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.utils.dummy import create_dummy_header, create_dummy_table


class MetisLssMfModelImpl(MetisRecipeImpl):
    class InputSet(AtmProfileInputSetMixin, AtmLineCatInputSetMixin, LsfKernelInputSetMixin, PipelineInputSet):
        # ++++++++++++ Main input ++++++++++++
        # Default (Path #2 in DRLD Section CritAlg)
        class LssSciFlux1dInput(SinglePipelineInput):
            Item = LssSciFlux1d

        # Alternative (Path #3 in DRLD Section CritAlg)
        class LssStd1dInput(SinglePipelineInput):
            Item = LssStd1d

    ProductMfBestFitTable = MfBestFitTable

    #   Method for processing
    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""

        # TODO: Invoke molecfit here
        # TODO: Check whether the new mf writes out the best-fit param file
        header = create_dummy_header()
        table = create_dummy_table()
        return {
            self.ProductMfBestFitTable(header, table),
        }
