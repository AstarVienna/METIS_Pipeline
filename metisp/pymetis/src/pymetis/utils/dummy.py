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
from cpl.core import PropertyList as CplPropertyList, Property as CplProperty


def python_to_cpl_type(what: type) -> cpl.core.Type:
    """
    Convert a Python type to a CPL type.

    Parameters
    ----------
    what: type
        Python type to convert.

    Returns
    -------
    cpl.core.Type
        Converted type.

    Raises
    ------
    KeyError
        If the type is not a Python type convertible to a CPL type.
    """
    try:
        return {
            int: cpl.core.Type.INT,
            float: cpl.core.Type.DOUBLE,
            str: cpl.core.Type.STRING,
            list: cpl.core.Type.ARRAY,
        }[what]
    except KeyError as exc:
        raise TypeError(f"Type {what} cannot be converted to a CPL type.") from exc


def make_cpl_property(name: str, value: Any) -> cpl.core.Property:
    """
    Create a new CPL Property from a Python variable.

    Parameters
    ----------
    name: string
        Name of the new property
    value:
        Python variable

    Returns
    -------
    cpl.core.Property
        New CPL Property

    Raises
    ------
    KeyError
        If the type of the variable is not a Python type convertible to a CPL type.

    """
    return CplProperty(name, python_to_cpl_type(type(value)), value)


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
    return cpl.core.Table.empty(rows)
