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

from typing import Optional, Any, final, Self

import cpl

from pymetis.classes.mixins.base import ParametrizableItem
from pymetis.utils.property import python_to_cpl_type


class QcParameter(ParametrizableItem):
    _name_template = "none"
    _class = None
    _type = None
    _description_template = "Mean level of the frame"
    _comment = None

    def __init__(self, value: Any):
        assert isinstance(value, self._type), \
            (f"{self.__class__.__qualname__} expected a {self._type} value, "
             f"but got {value} ({type(value)}) instead")
        self._value = value

    @property
    def value(self) -> Any:
        return self._value

    @classmethod
    def extended_description_line(cls) -> str:
        """
        Return a formatted description line for the man page.
        """
        # [5:] is there to get rid of "Type." prefix
        return f"    {cls.name():<31s} {f'{python_to_cpl_type(cls._type)}'[5:]:<14s} {cls.description()}"

    def as_property(self) -> cpl.core.Property:
        return cpl.core.Property(self.name(), self._type, self.value, self.description())
