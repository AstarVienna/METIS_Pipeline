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

from pymetis.engine.inputs import SinglePipelineInput, MultiplePipelineInput
from pymetis.engine.inputs.mixins import OptionalInputMixin, PrimaryInputMixin
from pymetis.instruments.metis import dataitems


"""
This file contains various ready-to-use `PipelineInput` classes.
They are often reused throughout many recipes and it is nice to have them clumped together.

In case they are not enough, you are welcome to add your own. Mind that

You should never derive actual `Input`s directly from `PipelineInput`, but rather from

 -  `SinglePipelineInput` (for `Input` classes with a single Frame)
 -  `MultiplePipelineInput` (for `Input` classes with a FrameSet)

You can override class attributes:

 -  `Item`
    points to the data item inside this `Input`
 -  `_required`
    A boolean telling the recipe if this input is required or not. Default is `True`, so most of the time
    it does not have to be touched. For optional inputs it is enough to define `_required = False`,
    or even better, derive from `OptionalInputMixin` first.
"""


class RawInput(PrimaryInputMixin, MultiplePipelineInput, ABC):
    Item = dataitems.Raw


class MasterDarkInput(SinglePipelineInput):
    Item = dataitems.MasterDark


class MasterFlatInput(SinglePipelineInput):
    Item = dataitems.MasterFlat


class LinearityInput(SinglePipelineInput):
    Item = dataitems.LinearityMap


class BadPixMapInput(SinglePipelineInput):
    Item = dataitems.BadPixMap


class PersistenceMapInput(SinglePipelineInput):
    Item = dataitems.PersistenceMap


class OptionalPersistenceMapInput(OptionalInputMixin, SinglePipelineInput):
    Item = dataitems.PersistenceMap


class GainMapInput(SinglePipelineInput):
    Item = dataitems.GainMap


class DistortionTableInput(SinglePipelineInput):
    Item = dataitems.DistortionTable


class WavecalInput(SinglePipelineInput):
    Item = dataitems.IfuWavecal


class PinholeTableInput(SinglePipelineInput):
    Item = dataitems.PinholeTable


class FluxstdCatalogInput(SinglePipelineInput):
    Item = dataitems.FluxStdCatalog


class MasterRsrfInput(SinglePipelineInput):
    Item = dataitems.MasterLssRsrf


class FluxCalTableInput(SinglePipelineInput):
    Item = dataitems.FluxCalTable


class LsfKernelInput(SinglePipelineInput):
    Item = dataitems.LsfKernel


class AtmProfileInput(SinglePipelineInput):
    Item = dataitems.AtmProfile


class AtmLineCatInput(SinglePipelineInput):
    Item = dataitems.AtmLineCatalog


class LaserTableInput(SinglePipelineInput):
    Item = dataitems.LaserTable


class SynthTransInput(SinglePipelineInput):
    Item = dataitems.SynthTrans


class WcuOffInput(RawInput):
    Item = dataitems.WcuOffRaw

