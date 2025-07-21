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

import cpl

from pymetis.classes.dataitems import TableDataItem
from pymetis.dataitems.raw import Raw
from pymetis.classes.mixins import BandLmMixin, BandNMixin, BandSpecificMixin


class AdcSlitloss(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template = r'{band}_ADC_SLITLOSS'
    _title_template = "{band} ADC slit loss"
    _description_template = "Table with ADC induced {band} slitlosses"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.SLIT'}


class LmAdcSlitloss(BandLmMixin, AdcSlitloss):
    pass


class NAdcSlitloss(BandNMixin, AdcSlitloss):
    pass


class AdcSlitlossRaw(BandSpecificMixin, Raw, abstract=True):
    #_name_template = r'{band}_ADC_SLITLOSSES_RAW'
    _name_template = r'{band}_ADC_SLITLOSS_RAW'
    _title_template = r'{band} ADC slit loss raw'
    _description_template = "Raw files for ADC slitloss determination (TBD)."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL


class LmAdcSlitlossRaw(BandLmMixin, AdcSlitlossRaw):
    pass


class NAdcSlitlossRaw(BandNMixin, AdcSlitlossRaw):
    pass