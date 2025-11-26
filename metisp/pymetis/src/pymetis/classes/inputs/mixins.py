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

from .inputset import PipelineInputSet
from .common import (GainMapInput, LinearityInput, BadPixMapInput,
                     AtmLineCatInput, AtmProfileInput, LsfKernelInput, WcuOffInput, OptionalPersistenceMapInput)


class PersistenceInputSetMixin(PipelineInputSet):
    class     PersistenceMapInput(OptionalPersistenceMapInput):
        pass


class GainMapInputSetMixin(PipelineInputSet):
    class GainMapInput(GainMapInput):
        pass


class LinearityInputSetMixin(PipelineInputSet):
    class LinearityInput(LinearityInput):
        pass


class BadPixMapInputSetMixin(PipelineInputSet):
    class BadPixMapInput(BadPixMapInput):
        pass


class AtmLineCatInputSetMixin(PipelineInputSet):
    class AtmLineCatInput(AtmLineCatInput):
        pass


class AtmProfileInputSetMixin(PipelineInputSet):
    class AtmProfileInput(AtmProfileInput):
        pass


class LsfKernelInputSetMixin(PipelineInputSet):
    class LsfKernelInput(LsfKernelInput):
        pass


class WcuOffInputSetMixin(PipelineInputSet):
    class WcuOffInput(WcuOffInput):
        pass
