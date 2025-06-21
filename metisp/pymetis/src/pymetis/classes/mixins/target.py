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

from abc import ABC

from pymetis.classes.mixins.base import Mixin


class TargetSpecificMixin(Mixin, ABC):
    """
    Mixin class for data items that need to define the `target` attribute.

    Hopefully it does not cause any issues with the MRO.
    """
    _target: str = None

    @classmethod
    def target(cls) -> str:
        return cls._target or r'{target}'

    @classmethod
    def get_target_string(cls) -> str:
        """
        Return a pretty formatted target string for human-oriented output.
        """
        return {
            'SCI': 'science target',
            'STD': 'standard star',
            'LAMP': 'lamp',
            'TWILIGHT': 'twilight',
        }.get(cls.target(), cls.target())


class TargetStdMixin(TargetSpecificMixin):
    _target: str = r'STD'


class TargetSciMixin(TargetSpecificMixin):
    _target: str = r'SCI'


class TargetSkyMixin(TargetSpecificMixin):
    _target: str = r'SKY'


class TargetLampMixin(TargetSpecificMixin):
    _target: str = r'LAMP'


class TargetTwilightMixin(TargetSpecificMixin):
    _target: str = r'TWILIGHT'
