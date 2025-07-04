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
from pymetis.classes.dataitems.adc.adc import AdcSlitloss, AdcSlitlossRaw
from pymetis.classes.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, \
    BadPixMapInputSetMixin, RawInput
from pymetis.classes.prefab import RawImageProcessor


class MetisAdcSlitlossImpl(RawImageProcessor):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, BadPixMapInputSetMixin,
                   RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = AdcSlitlossRaw

    ProductAdcSlitloss = AdcSlitloss

    # =========================================================================================
    #    Methods
    # =========================================================================================

    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""
        header = self._create_dummy_header()
        table = self._create_dummy_table()
        return {
            self.ProductAdcSlitloss(header, table),
        }

