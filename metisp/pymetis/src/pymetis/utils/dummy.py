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
from typing import Any

import cpl
import numpy as np
from astropy.table import QTable
from cpl.core import PropertyList as CplPropertyList, Property as CplProperty

from pymetis.utils.property import make_cpl_property


def create_dummy_header(**kwargs) -> cpl.core.PropertyList:
    """
    Create a dummy header (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    return cpl.core.PropertyList([
        make_cpl_property(name, prop) for name, prop in kwargs.items()
    ])


def create_dummy_image(size: int = 2048, dtype: cpl.core.Type = cpl.core.Type.FLOAT) -> cpl.core.Image:
    """
    Create a dummy image (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    return cpl.core.Image.zeros(size, size, dtype)


def create_dummy_table(rows: int = 3) -> cpl.core.Table:
    """
    Create a dummy table (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    table = QTable()
    table['data'] = np.array([5, 6, 7])
    return cpl.core.Table(table)
