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

from pymetis.classes.dataitems.dataitem import ImageDataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, BandSpecificMixin, \
    BandLmMixin, SourceLampMixin, BandNMixin, SourceSpecificMixin, DetectorSpecificMixin


class MasterFlat(DetectorSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'MASTER_FLAT_{detector}'
    _title_template = r'{detector} master flat'
    _description_template = "Abstract base class for master flats. Please subclass."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}


class MasterFlat2rg(Detector2rgMixin, MasterFlat):
    pass


class MasterFlatGeo(DetectorGeoMixin, MasterFlat):
    pass


class MasterFlatIfu(DetectorIfuMixin, MasterFlat):
    pass


class MasterImgFlat(BandSpecificMixin, SourceSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'MASTER_IMG_FLAT_{source}_{band}'
    _title_template = r'{band} {source} master flat'
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

    @classmethod
    def description(cls):
        return f"Master flat frame for {cls.band()} data"


class MasterImgFlatLampLm(BandLmMixin, SourceLampMixin, MasterImgFlat):
    pass


class MasterImgFlatLampN(BandNMixin, SourceLampMixin, MasterImgFlat):
    pass
