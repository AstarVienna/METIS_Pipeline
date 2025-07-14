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

from pymetis.classes.dataitems.raw import Raw
from pymetis.classes.mixins import BandSpecificMixin, BandLmMixin, BandNMixin


class LssWaveRaw(BandSpecificMixin, Raw, abstract=True):
    _name_template = r'{band}_LSS_WAVE_RAW'
    _title_template = "{band} LSS wave raw"
    _description_template = "Raw LSS spectra of the WCU lasers in {band} band"
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE'}


class LmLssWaveRaw(BandLmMixin, LssWaveRaw):
    pass


class NLssWaveRaw(BandNMixin, LssWaveRaw):
    pass