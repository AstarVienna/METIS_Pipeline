"""
This file is part of an A* Pipeline.
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
import numbers
from types import NoneType
from typing import Any, ClassVar, Self

import cpl

from ..core.parametrizable import ParametrizableItem
from pymetis.engine.core.functions.property import python_to_cpl_type


class QcParameter(ParametrizableItem, abstract=True):
    """
    An encapsulation of a QC parameter, as specified in the DRLD.
    """

    _name_template: ClassVar[str] = "none"
    _type: ClassVar[type] = NoneType
    _unit: ClassVar[str | None] = "undefined"   # None means dimensionless; see tests/test_qc_units.py for the vocabulary
    _default: ClassVar[Any] = None
    _description_template: ClassVar[str] = "<no description provided>"
    _comment: ClassVar[str] = ""

    _registry: ClassVar[dict[str, type[Self]]] = {}       # fully resolved name -> concrete class
    _templates: ClassVar[dict[str, type[Self]]] = {}      # name with placeholders -> hand-written template

    def __init__(self, value: Any):
        # A value needs a fully resolved name to be written under; a parameter that is not
        # available (None) is never written, so an index placeholder (`LCOEFF{order}`) may stay.
        if value is not None and '{' in self.name():
            raise TypeError(f"{self.__class__.__qualname__}: QC name {self.name()!r} still has placeholders; "
                            f"specialize it first (e.g. `.specialized(order=1)`)")
        self._value = None if value is None else self._coerce(value)

    @classmethod
    def _coerce(cls, value: Any) -> Any:
        """
        Bring a value into the declared Python type: algorithms hand over numpy scalars, and
        an integer is a perfectly good float. A bool is never a count, and nothing else is
        converted silently.
        """
        if cls._type is int and isinstance(value, numbers.Integral) and not isinstance(value, bool):
            return int(value)
        if cls._type is float and isinstance(value, numbers.Real) and not isinstance(value, bool):
            return float(value)
        if cls._type is str and isinstance(value, str):
            return str(value)
        if isinstance(value, cls._type) and not (cls._type is int and isinstance(value, bool)):
            return value
        raise ValueError(f"{cls.__qualname__} expected a {cls._type.__name__} value, "
                         f"but got {value!r} ({type(value).__name__}) instead")

    @property
    def available(self) -> bool:
        """ False when the recipe could not determine the value (``None``); such a parameter is reported, not written. """
        return self._value is not None

    def __str__(self):
        return f"{self.name()} = {self.value!s}"

    @property
    def value(self) -> Any:
        return self._value

    @classmethod
    def extended_description_line(cls) -> str:
        """
        Return a formatted description line for the man page.
        """
        name = f"{cls.name():<35s}"
        unit_default = f"[{str(cls._unit)}, default {str(cls._default)}]"
        description = f"{cls.description():<60}"

        # [5:] is there to get rid of "Type." prefix
        return (f"{name} {f'{python_to_cpl_type(cls._type)}'[5:]:<14s} "
                f"{unit_default:<32s}"
                f"{description} ")

    def as_property(self) -> cpl.core.Property | None:
        if not self.available:
            return None
        return cpl.core.Property(self.name(), python_to_cpl_type(self._type), self.value, self.description())
