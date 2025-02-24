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

import cpl.ui

from pymetis.prefab.rawimage import RawImageProcessor


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    DarkImageProcessor is a subclass of RawImageProcessor that:

     1. takes a set of raw images to combine
     2. requires a single `master_dark` frame, that will be subtracted from every raw image
     3. combines the raws after subtraction into a single product

    Also provides methods for loading and verification of the dark frame,
    warns if multiple master darks are provided, etc.
    """
    class InputSet(RawImageProcessor.InputSet):
        """
        A DarkImageProcessor's Input is just a raw image processor input with a master dark frame.
        """
        MasterDarkInput: type = NotImplemented
