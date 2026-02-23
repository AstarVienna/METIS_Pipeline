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

from .input import PipelineInput
from .single import SinglePipelineInput
from .multiple import MultiplePipelineInput
from .mixins import PersistenceInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin, BadPixMapInputSetMixin

from .common import (RawInput,
                     MasterDarkInput,
                     MasterFlatInput,
                     LinearityInput,
                     BadPixMapInput,
                     PersistenceMapInput,
                     GainMapInput,
                     FluxCalTableInput,
                     FluxstdCatalogInput,
                     PinholeTableInput,
                     DistortionTableInput,
                     LsfKernelInput,
                     AtmProfileInput,
                     AtmLineCatInput,
                     WavecalInput,
                     OptionalInputMixin,
                     LaserTableInput,
                     SynthTransInput, MasterRsrfInput)

__all__ = [
    'PipelineInputSet',
    'PipelineInput', 'SinglePipelineInput', 'MultiplePipelineInput',
    'RawInput', 'MasterDarkInput', 'MasterFlatInput', 'LinearityInput', 'BadPixMapInput',
    'PersistenceMapInput', 'GainMapInput', 'FluxCalTableInput', 'FluxstdCatalogInput',
    'PinholeTableInput', 'DistortionTableInput', 'LsfKernelInput', 'AtmProfileInput', 'MasterRsrfInput',
    'WavecalInput', 'OptionalInputMixin',
    'LsfKernelInput', 'AtmLineCatInput', 'LaserTableInput', 'SynthTransInput',
    'PersistenceInputSetMixin', 'GainMapInputSetMixin', 'LinearityInputSetMixin', 'BadPixMapInputSetMixin',
]
