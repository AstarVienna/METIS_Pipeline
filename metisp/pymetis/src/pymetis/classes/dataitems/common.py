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

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins.band import BandLmMixin, BandNMixin, BandIfuMixin
from pymetis.classes.mixins.target import TargetSpecificMixin


class PersistenceMap(DataItem):
    _title = "persistence map"
    _name = "PERSISTENCE_MAP"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "Persistence map"
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PERSISTENCE_MAP'


class FluxCalTable(DataItem):
    _title = "flux table"
    _name = "FLUXCAL_TAB"
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _description = "Conversion between instrumental and physical flux units"
    _oca_keywords = {'PRO.CATG'}


class PinholeTable(DataItem):
    _title = "pinhole table"
    _name = r'PINHOLE_TABLE'
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "Table of pinhole locations"
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PINHOLE_TABLE'


class AtmProfile(DataItem):
    _name = r'ATM_PROFILE'
    _title = "atmosphere profile"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = ("Atmospheric profile containing height information on temperature, "
                    "pressure and molecular abundances")


class LsfKernel(DataItem):
    _name = r'LSF_KERNEL'
    _title = "line spread function kernel"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "Wavelength dependent model of the LSF"


class FluxStdCatalog(DataItem):
    _name = r'FLUXSTD_CATALOG'
    _title = "catalog of standard stars"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description = "Catalog of standard stars"
    _oca_keywords = set()


class IfuScienceCubeCalibrated(BandIfuMixin, DataItem):
    _name = r'IFU_SCI_CUBE_CALIBRATED'
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _title = "IFU science cube calibrated"
    _description = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class AtmLineCatalog(DataItem):
    _name = r'ATM_LINE_CAT'
    _oca_keywords = {'PRO.CATG'}


class IfuTelluric(DataItem):
    _name = r'IFU_TELLURIC'
    _title = "Telluric correction"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_type = cpl.ui.Frame.FrameType.TABLE
    _description = "Transmission function for the telluric correction."
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}