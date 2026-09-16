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
    Mixin for the inputs whose frames CPL should treat as the raw material of the
    recipe, as opposed to the calibrations applied to them: they are stamped
    `FrameGroup.RAW`, so CPL DFS inherits the product header from the first of them and
    lists them as the raw frames of the product. Instrument raws and pipeline products
    alike may play this role, and a recipe may have several such inputs (e.g. the WCU
    OFF frames next to the lamp frames); every recipe should have at least one.
    """
    _group = cpl.ui.Frame.FrameGroup.RAW
