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

from pymetis.classes.dataitems import Raw
from pymetis.classes.mixins import BandLmMixin, BandNMixin


class LssRsrfRaw(Raw, abstract=True):
    _name_template = r'{band}_LSS_RSRF_RAW'
    _title_template = "{band} LSS RSRF raw"
    _description_template = "Raw exposure of the WCU flat field lamp through the LSS to achieve the RSRF."
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                     'INS.OPTI3.NAME', 'INS.OPTI12.NAME', 'INS.OPTI13.NAME',
                     'DRS.SLIT'}


class LmLssStdRaw(BandLmMixin, LssRsrfRaw):
    pass


class NLssStdRaw(BandNMixin, LssRsrfRaw):
    pass
