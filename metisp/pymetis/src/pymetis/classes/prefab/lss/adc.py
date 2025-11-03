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
from pymetis.dataitems.adc.adc import AdcSlitloss, AdcSlitlossRaw
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, \
    BadPixMapInputSetMixin, RawInput
from pymetis.classes.inputs.mixins import WcuOffInputSetMixin
from pymetis.classes.prefab import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_header, create_dummy_table


class MetisAdcSlitlossImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, BadPixMapInputSetMixin,
                   WcuOffInputSetMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = AdcSlitlossRaw

    ProductAdcSlitloss = AdcSlitloss

    # =========================================================================================
    #    Methods
    # =========================================================================================

    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""

        raws = self.inputset.raw.load_data('DET1.DATA')
        primary_header = self.inputset.raw.items[0].primary_header

        header = create_dummy_header()
        table = create_dummy_table()
        return {
            self.ProductAdcSlitloss(
                primary_header,
                Hdu(header, table, name='TABLE')
            ),
        }

