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

from pymetis.dataitems import ImageDataItem
from pymetis.classes.mixins import (BandSpecificMixin, BandNMixin, BandLmMixin,
                                    TargetSpecificMixin, TargetStdMixin, TargetSciMixin)


class ObjectCatalog(BandSpecificMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_{target}_OBJECT_CAT'
    _title_template = "{band} {target} object catalog"
    _description_template = "Catalog of masked objects in {target} {band} exposures"
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB


class LmStdObjectCatalog(BandLmMixin, TargetStdMixin, ObjectCatalog):
    pass


class NStdObjectCatalog(BandNMixin, TargetStdMixin, ObjectCatalog):
    pass


class LmSciObjectCatalog(BandLmMixin, TargetSciMixin, ObjectCatalog):
    pass


class NSciObjectCatalog(BandNMixin, TargetSciMixin, ObjectCatalog):
    pass
