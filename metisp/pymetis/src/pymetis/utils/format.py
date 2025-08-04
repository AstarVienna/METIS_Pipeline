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

from typing import Optional


class FormatPlaceholder:
    def __init__(self, key):
        self.key = key

    def __format__(self, spec):
        value = f'{self.key}{f':{spec}' if spec else ''}'
        return f"{{{value}}}"


class FormatDict(dict):
    def __missing__(self, key):
        return FormatPlaceholder(key)


def partial_format(template: str, **kwargs) -> str:
    return template.format_map(FormatDict(**kwargs))

