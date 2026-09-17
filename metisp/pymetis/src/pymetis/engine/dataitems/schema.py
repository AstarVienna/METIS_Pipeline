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
from cpl.core import Image, ImageList, Table


def detectors(kind: type[Image | Table | ImageList], count: int, extension: str = 'DATA') -> dict:
    """
    The schema of an item with one `DET<n>.<extension>` HDU of `kind` per detector,
    e.g. ``detectors(Image, 4)`` for the four IFU detectors.
    """
    return {'PRIMARY': None} | {f'DET{n}.{extension}': kind for n in range(1, count + 1)}
