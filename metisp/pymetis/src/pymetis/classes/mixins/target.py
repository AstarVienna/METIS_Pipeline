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

from pymetis.classes.mixins.base import Parametrizable


class TargetSpecificMixin(Parametrizable):
    """
    Mixin class for data items that need to define the `target` attribute.

    Hopefully it does not cause any issues with the MRO.
    """
    _target: str = None

    def __init_subclass__(cls, *, target=None, **kwargs):
        if target is not None:
            cls._target = target
        super().__init_subclass__(**kwargs)

    @classmethod
    def target(cls) -> str:
        return cls._target

    @classmethod
    def get_target_string(cls) -> str:
        """
        Return a pretty formatted target string for human-oriented output.
        """
        return {
            'SCI': 'science object',
            'STD': 'standard star',
            'SKY': 'sky',
        }.get(cls.target(), cls.target())

    @classmethod
    def tag_parameters(cls):
        return super().tag_parameters() | {'target': cls._target}


class TargetStdMixin(TargetSpecificMixin, target='STD'):
    pass


class TargetSciMixin(TargetSpecificMixin, target='SCI'):
    pass


class TargetSkyMixin(TargetSpecificMixin, target='SKY'):
    pass
