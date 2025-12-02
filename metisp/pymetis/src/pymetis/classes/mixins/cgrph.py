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

from pymetis.classes.mixins.base import KeywordMixin


class CgrphSpecificMixin(KeywordMixin, keyword='cgrph'):
    _cgrph: str = None

    @classmethod
    def cgrph(cls) -> str:
        return cls._cgrph

    def __init_subclass__(cls, *, cgrph=None, **kwargs):
        if cgrph is not None:
            cls._cgrph = cgrph
        super().__init_subclass__(**kwargs)

    @classmethod
    def tag_parameters(cls):
        return super().tag_parameters() | {'cgrph': cls._cgrph}


class CgrphRavcMixin(CgrphSpecificMixin, cgrph='RAVC'):
    pass


class CgrphCvcMixin(CgrphSpecificMixin, cgrph='CVC'):
    pass


class CgrphAppMixin(CgrphSpecificMixin, cgrph='APP'):
    pass

