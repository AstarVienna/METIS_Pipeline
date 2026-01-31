"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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
from pymetis.classes.dataitems.productset import PipelineProductSet
from pymetis.dataitems.lss.rsrf import LssRsrfPinholeRaw, MasterLssRsrf
from pymetis.dataitems.lss.trace import LssTrace
from pymetis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.inputs import SinglePipelineInput, RawInput, PersistenceMapInput, OptionalInputMixin, GainMapInput, \
    LinearityInput, BadPixMapInput
from pymetis.classes.prefab import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header, create_dummy_table


class MetisLssTraceImpl(DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):

        class RawInput(RawInput):
            Item = LssRsrfPinholeRaw

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass

        class BadPixMapInput(BadPixMapInput):
            pass

        class LmRsrfWcuOffInput(RawInput):
            Item = WcuOffRaw

        class MasterRsrfInput(SinglePipelineInput):
            Item = MasterLssRsrf

    class ProductSet(PipelineProductSet):
        TraceTable = LssTrace

    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""
        raws = self.inputset.raw.load_data('DET1.DATA')
        master_rsrf = self.inputset.master_rsrf.load_data('DET1.DATA')

        primary_header = self.inputset.master_rsrf.item.primary_header

        trace_tab_header = create_dummy_header()
        trace_tab_data = create_dummy_table()

        return {
            self.ProductSet.TraceTable(
                primary_header,
                Hdu(trace_tab_header, trace_tab_data, name='TABLE'))
        }

