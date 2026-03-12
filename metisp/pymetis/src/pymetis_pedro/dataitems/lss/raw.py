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

from pymetis.dataitems.raw import Raw
from pymetis.classes.mixins import BandLmMixin, TargetStdMixin, BandNMixin, TargetSciMixin, BandSpecificMixin


class LssRaw(BandSpecificMixin, Raw, abstract=True):
    _name_template = r'{band}_LSS_{target}_RAW'
    _title_template = "{band} LSS {target} raw"
    _description_template = "{band}-band long-slit spectroscopy raw exposure of a {target}"
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                     'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME',
                     'DRS.SLIT'}


class LssStdRaw(TargetStdMixin, LssRaw, abstract=True):
    pass


class LmLssStdRaw(BandLmMixin, LssStdRaw):
    pass


class NLssStdRaw(BandNMixin, LssStdRaw):
    pass


class LssSciRaw(TargetSciMixin, LssRaw, abstract=True):
    pass


class LmLssSciRaw(BandLmMixin, LssSciRaw):
    pass


class NLssSciRaw(BandNMixin, LssSciRaw):
    pass