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

from pymetis.inputs import PersistenceMapInput, PipelineInputSet


class Detector2rgMixin:
    detector: str = '2RG'
    band: str = 'LM'

class DetectorGeoMixin:
    detector: str = 'GEO'
    band: str = 'N'

class DetectorIfuMixin:
    detector: str = 'IFU'

class TargetSciMixin:
    target: str = 'SCI'

class TargetStdMixin:
    target: str = 'STD'



class PersistenceInputSetMixin(PipelineInputSet):
    def __init__(self, frameset: cpl.ui.FrameSet):
        self.persistence_map = PersistenceMapInput(frameset)
        super().__init__(frameset)
        self.inputs += [self.persistence_map]