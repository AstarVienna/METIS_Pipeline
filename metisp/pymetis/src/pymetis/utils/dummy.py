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


def create_dummy_header() -> cpl.core.PropertyList:
    """
    Create a dummy header (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    return cpl.core.PropertyList()


def create_dummy_image() -> cpl.core.Image:
    """
    Create a dummy image (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    return cpl.core.Image.zeros(32, 32, cpl.core.Type.FLOAT)


def create_dummy_table() -> cpl.core.Table:
    """
    Create a dummy table (absolutely no assumptions, just to have something to work with).
    # ToDo This function should not survive in the future.
    """
    return cpl.core.Table.empty(3)
