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

from pymetis.classes.dataitem import ImageDataItem
from .wcuoff import WcuOffRaw, LmWcuOffRaw, NWcuOffRaw, IfuWcuOffRaw


class Raw(ImageDataItem, abstract=True):
    """
    Abstract intermediate class for all raw items.
    """
    _name_template = r'RAW'
    _title_template = "abstract raw"
    _description_template = "Abstract base class for all raw inputs. Please subclass."
    _frame_group = cpl.ui.Frame.FrameGroup.RAW


__all__ = [
    'Raw',
    'WcuOffRaw', 'LmWcuOffRaw', 'NWcuOffRaw', 'IfuWcuOffRaw',
]