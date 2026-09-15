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
import cpl

from abc import ABC

from pymetis.engine.inputs import PipelineInput


class OptionalInputMixin(PipelineInput, ABC):
    """
    Mixin for inputs that are optional -- the recipe can proceed without them and still produce meaningful output.
    Prefer using this to setting `_required = False` directly in the class.
    """
    _required = False


class PrimaryInputMixin(PipelineInput, ABC):
    """
    Mixin for the input a recipe actually reduces, as opposed to the calibrations it
    applies: its frames are stamped `FrameGroup.RAW`, so CPL DFS inherits the product
    header from them and lists them as the raw frames of the product. Instrument raws
    and pipeline products alike may play this role.
    """
    _group = cpl.ui.Frame.FrameGroup.RAW
