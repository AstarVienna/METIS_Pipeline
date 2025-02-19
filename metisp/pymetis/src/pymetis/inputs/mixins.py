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

import cpl.ui

from pymetis.inputs import PersistenceMapInput, PipelineInputSet, GainMapInput, LinearityInput


class PersistenceInputSetMixin(PipelineInputSet):
    PersistenceMapInput = PersistenceMapInput

    def __init__(self, frameset: cpl.ui.FrameSet):
        super().__init__(frameset)

        self.persistence_map = PersistenceMapInput(frameset, required=False)
        self.inputs |= {self.persistence_map}


class GainMapInputSetMixin(PipelineInputSet):
    def __init__(self, frameset: cpl.ui.FrameSet):
        super().__init__(frameset)
        self.gain_map = GainMapInput(frameset)
        self.inputs |= {self.gain_map}


class LinearityInputSetMixin(PipelineInputSet):
    def __init__(self, frameset: cpl.ui.FrameSet):
        super().__init__(frameset)
        self.linearity = LinearityInput(frameset)
        self.inputs |= {self.linearity}
