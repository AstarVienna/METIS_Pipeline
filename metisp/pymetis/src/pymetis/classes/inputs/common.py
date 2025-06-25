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

import re

from abc import ABC
from typing import Pattern

import cpl

from . import PipelineInput
from .single import SinglePipelineInput
from .multiple import MultiplePipelineInput

from ..dataitems.dataitem import DataItem
from ..dataitems.common import (PersistenceMap, LinearityMap, FluxCalTable, PinholeTable, AtmProfile, LsfKernel,
                                FluxStdCatalog)
from ..dataitems.raw import Raw
from ..dataitems.badpixmap import BadPixMap
from pymetis.classes.dataitems.distortion.table import DistortionTable
from ..dataitems.gainmap import GainMap
from pymetis.classes.dataitems.masterdark.masterdark import MasterDark
from ..dataitems.masterflat import MasterFlat
from ..dataitems.wavecal import IfuWavecal

"""
This file contains various ready-to-use `PipelineInput` classes.
You should never derive directly from `PipelineInput`, but rather from

 -  `SinglePipelineInput` (for `Input` classes with a single Frame)
 -  `MultiplePipelineInput` (for `Input` classes with a FrameSet)

You should override class attributes:

 -  `_title`
    The short description of an `Input` class (this is only used for log output).
 -  `_group`
    The `cpl.ui.Frame.FrameGroup` property that is required by PyEsoRex and also by CPL.
 -  `_tags`
    The list of tags that are accepted by this input. Note that in some classes it is not defined yet, but definition
    is deferred to further children (but has to be defined *somewhere*). Tags can contain format template placeholders,
    which will be filled from **kwargs in the __init__ method, to allow various detectors or bands.
 -  `_required`
    A boolean telling the recipe if this input is required or not. Default is True, so it is enough to say
    `_required = False` for optional inputs.
"""


class OptionalInputMixin(PipelineInput, ABC):
    _required: bool = False     # Many inputs are by default optional, this mixin provides that


class RawInput(MultiplePipelineInput, ABC):
    Item: type[DataItem] = Raw
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW


class MasterDarkInput(SinglePipelineInput):
    Item: type[DataItem] = MasterDark
    _tags: Pattern = re.compile(r"MASTER_DARK_(?P<detector>2RG|GEO|IFU)")


class MasterFlatInput(SinglePipelineInput):
    Item: type[DataItem] = MasterFlat
    _tags: Pattern = re.compile(r"MASTER_IMG_FLAT_LAMP_(?P<band>LM|N)")


class LinearityInput(SinglePipelineInput):
    Item: type[DataItem] = LinearityMap
    _tags: Pattern = re.compile(r"LINEARITY_(?P<detector>2RG|GEO|IFU)")


class BadpixMapInput(SinglePipelineInput):
    Item: type[DataItem] = BadPixMap
    _tags: Pattern = re.compile(r"BADPIX_MAP_(?P<detector>2RG|GEO|IFU)")


class PersistenceMapInput(SinglePipelineInput):
    Item: type[DataItem] = PersistenceMap
    _tags: Pattern = re.compile(r"PERSISTENCE_MAP")
    _required = False           # By default, persistence maps are optional


class GainMapInput(SinglePipelineInput):
    Item: type[DataItem] = GainMap
    _tags: Pattern = re.compile(r"GAIN_MAP_(?P<detector>2RG|GEO|IFU)")


class DistortionTableInput(SinglePipelineInput):
    Item: type[DataItem] = DistortionTable
    _tags: Pattern = re.compile(r"IFU_DISTORTION_TABLE")


class WavecalInput(SinglePipelineInput):
    Item: type[DataItem] = IfuWavecal
    _title: str = "wavelength calibration"
    _tags: Pattern = re.compile(r"IFU_WAVECAL")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Image with wavelength at each pixel"


class PinholeTableInput(SinglePipelineInput):
    Item: type[DataItem] = PinholeTable
    _tags: Pattern = re.compile(r"PINHOLE_TABLE")

class FluxstdCatalogInput(SinglePipelineInput):
    Item: type[DataItem] = FluxStdCatalog
    _tags: Pattern = re.compile(r"FLUXSTD_CATALOG")


class FluxCalTableInput(SinglePipelineInput):
    Item: type[DataItem] = FluxCalTable
    _tags: re.Pattern = re.compile(r"FLUXCAL_TAB")


class LsfKernelInput(SinglePipelineInput):
    Item: type[DataItem] = LsfKernel
    _tags: Pattern = re.compile(r"LSF_KERNEL")


class AtmProfileInput(SinglePipelineInput):
    Item: type[DataItem] = AtmProfile
    _tags: Pattern = re.compile(r"ATM_PROFILE")
