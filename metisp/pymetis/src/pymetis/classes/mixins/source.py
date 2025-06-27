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

from pymetis.classes.mixins.base import Mixin


class SourceSpecificMixin(Mixin):
    """
    Mixin class for data items that need to define the `target` attribute.

    Hopefully it does not cause any issues with the MRO.
    """
    _source: str = None

    @classmethod
    def source(cls) -> str:
        return cls._source

    @classmethod
    def get_source_string(cls) -> str:
        """
        Return a pretty formatted target string for human-oriented output.
        """
        return {
            'LAMP': 'lamp',
            'TWILIGHT': 'twilight',
        }.get(cls.source(), cls.source())


class SourceLampMixin(SourceSpecificMixin):
    _source: str = r'LAMP'


class SourceTwilightMixin(SourceSpecificMixin):
    _source: str = r'TWILIGHT'
