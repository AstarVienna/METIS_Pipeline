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
from pymetis.classes.mixins import BandLmMixin, BandNMixin


class SynthTrans(TableDataItem, abstract=True):
    _name_template = r'{band}_SYNTH_TRANS'
    _title_template = "{band} synthetic transmission"
    _description_template = "Synthetic {band} transmission used for default telluric correction of STD stars"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL


class LmSynthTrans(BandLmMixin, SynthTrans):
    pass


class NSynthTrans(BandNMixin, SynthTrans):
    pass


class LssSynthTrans(TableDataItem, abstract=True):
    """
    Transmission spectrum
    """
    _name_template = r'{band}_LSS_SYNTH_TRANS'
    _title_template = "{band} LSS synthetic transmision curve"
    _description_template = ("Synthetic {band} transmission curve to be used"
                             "for telluric correction of flux standard stars.")
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL


class LmLssSynthTrans(BandLmMixin, LssSynthTrans):
    pass


class NLssSynthTrans(BandNMixin, LssSynthTrans):
    pass


