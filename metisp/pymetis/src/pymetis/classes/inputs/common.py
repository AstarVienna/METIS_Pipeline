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

from . import PipelineInput
from .single import SinglePipelineInput
from .multiple import MultiplePipelineInput

from ..dataitems.common import PersistenceMap, FluxCalTable, PinholeTable, AtmProfile, LsfKernel, FluxStdCatalog, \
    AtmLineCatalog, LaserTable
from ..dataitems.linearity.linearity import LinearityMap
from ..dataitems.raw import Raw
from ..dataitems.badpixmap import BadPixMap
from pymetis.classes.dataitems.distortion.table import DistortionTable
from ..dataitems.gainmap import GainMap
from pymetis.classes.dataitems.masterdark.masterdark import MasterDark
from ..dataitems.masterflat import MasterFlat
from ..dataitems.synth import SynthTrans
from ..dataitems.wavecal import IfuWavecal

"""
This file contains various ready-to-use `PipelineInput` classes.
You should never derive directly from `PipelineInput`, but rather from

 -  `SinglePipelineInput` (for `Input` classes with a single Frame)
 -  `MultiplePipelineInput` (for `Input` classes with a FrameSet)

You should override class attributes:

 -  `Item`
    points to the data item inside this `Input`
 -  `_required`
    A boolean telling the recipe if this input is required or not. Default is True, so it is enough to say
    `_required = False` for optional inputs, or even better, derive from `OptionalInputMixin` first
"""


class OptionalInputMixin(PipelineInput, ABC):
    _required = False     # Many inputs are by default optional, this mixin provides that


class RawInput(MultiplePipelineInput, ABC):
    Item = Raw


class MasterDarkInput(SinglePipelineInput):
    Item = MasterDark


class MasterFlatInput(SinglePipelineInput):
    Item = MasterFlat


class LinearityInput(SinglePipelineInput):
    Item = LinearityMap


class BadPixMapInput(SinglePipelineInput):
    Item = BadPixMap


class PersistenceMapInput(SinglePipelineInput):
    Item = PersistenceMap
    _required = False           # By default, persistence maps are optional


class GainMapInput(SinglePipelineInput):
    Item = GainMap


class DistortionTableInput(SinglePipelineInput):
    Item = DistortionTable


class WavecalInput(SinglePipelineInput):
    Item = IfuWavecal


class PinholeTableInput(SinglePipelineInput):
    Item = PinholeTable


class FluxstdCatalogInput(SinglePipelineInput):
    Item = FluxStdCatalog


class FluxCalTableInput(SinglePipelineInput):
    Item = FluxCalTable


class LsfKernelInput(SinglePipelineInput):
    Item = LsfKernel


class AtmProfileInput(SinglePipelineInput):
    Item = AtmProfile


class AtmLineCatInput(SinglePipelineInput):
    Item = AtmLineCatalog


class LaserTableInput(SinglePipelineInput):
    Item = LaserTable


class SynthTransInput(SinglePipelineInput):
    Item = SynthTrans

