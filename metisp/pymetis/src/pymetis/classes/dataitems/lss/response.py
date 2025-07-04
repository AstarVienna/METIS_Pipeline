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
from pymetis.classes.mixins import BandSpecificMixin, BandLmMixin, BandNMixin


class MasterResponse(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template = r'MASTER_{band}_RESPONSE'
    _title_template = r'master {band}-band response'
    _description_template = "Master {band}-band response function for absolute flux calibration"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'INS.MODE', 'INS.SPEC.SETUP', 'DRS.SLIT'}


class MasterLmResponse(BandLmMixin, MasterResponse):
    pass


class MasterNResponse(BandNMixin, MasterResponse):
    pass


class StdTransmission(TableDataItem):
    _name_template = r'STD_TRANSMISSION'
    _title_template = "standard transmission curve"
    _description_template = "Transmission curve derived by means of a standard star"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'INS.MODE', 'INS.SPEC.SETUP', 'DRS.SLIT'}
