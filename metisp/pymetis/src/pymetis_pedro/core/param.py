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
from typing import Self


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

    @classmethod
    def tag_parameters(cls) -> dict[str, str]:
        """
        Return the tag parameters for this class.
        By default, there are none, but mixins may add their own.
        """
        return {}


class KeywordMixin(Parametrizable, ABC):
    """
    Base class for keyword-parametrizable mixins.

    Contains a global registry of such classes, with placeholders as keys
    """

    # Global registry of parametrizable tags in the form {keyword: class},
    # e.g. {'detector': DetectorSpecificMixin, ...}
    # Filled automatically with __init_subclass__.
    _registry: dict[str, type[Self]] = {}

    def __init_subclass__(cls, *, keyword: str = None, **kwargs):
        if keyword is not None:
            cls._registry[keyword] = cls
        super().__init_subclass__(**kwargs)

    @classmethod
    def registry(cls) -> dict[str, type[Self]]:
        """ Class property to access the global registry """
        return cls._registry
