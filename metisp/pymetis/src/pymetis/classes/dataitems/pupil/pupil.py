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

from pymetis.classes.dataitems import DataItem
from pymetis.classes.mixins.band import BandSpecificMixin, BandLmMixin, BandNMixin


class PupilImagingReduced(BandSpecificMixin, DataItem, abstract=True):
    _name_template = r'{band}_PUPIL_REDUCED'
    _title_template = r"{band} pupil reduced"
    _description_template = "Reduced pupil image in {band} mode."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'DRS.PUPIL'}


class LmPupilImagingReduced(BandLmMixin, PupilImagingReduced):
    pass


class NPupilImagingReduced(BandNMixin, PupilImagingReduced):
    pass