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


class Parametrizable:
    """
    Base class for all types parametrizable by tags, such as
        - band: (LM|N|IFU)
        - target: (STD|SCI)
        - detector: (2RG|GEO|IFU)
        - source: (LAMP|TWILIGHT)

    Other parameters might be useful for different pipelines.

    Currently applies to DataItems, PipelineInputSets and their Mixins.
    """

    _tag_parameters: dict[str, str] = {}

    @classmethod
    def tag_parameters(cls) -> dict[str, str]:
        """
        Return the tag parameters for this class.
        By default, there are none, but mixins may add their own.
        """
        return cls._tag_parameters

    def __init_subclass__(cls, **kwargs):
        merged = {}
        for base in reversed(cls.__mro__):
            params = base.__dict__.get('_tag_parameters')
            if isinstance(params, dict):
                merged.update(params)

        merged.update(kwargs)

        cls._tag_parameters = merged
