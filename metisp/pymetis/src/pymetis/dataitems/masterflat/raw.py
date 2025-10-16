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
from cpl.core import Image

from pymetis.dataitems.raw import Raw
from pymetis.classes.mixins.band import BandNMixin, BandLmMixin, BandSpecificMixin
from pymetis.classes.mixins.source import SourceSpecificMixin, SourceLampMixin, SourceTwilightMixin


class FlatRaw(BandSpecificMixin, SourceSpecificMixin, Raw, abstract=True):
    _name_template = r'{band}_FLAT_{source}_RAW'
    _title_template = r'{band} flat {source} raw'
    _description_template = r'Flat raw'
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                     'INS.OPTI3.NAME', 'INS.OPTI12.NAME', 'INS.OPTI13.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'IMAGE': Image,
    }


class LmFlatLampRaw(BandLmMixin, SourceLampMixin, FlatRaw):
    pass


class LmFlatTwilightRaw(BandLmMixin, SourceTwilightMixin, FlatRaw):
    pass


class NFlatLampRaw(BandNMixin, SourceLampMixin, FlatRaw):
    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class NFlatTwilightRaw(BandNMixin, SourceTwilightMixin, FlatRaw):
    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

