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
from cpl.core import Image, Table

from pymetis.classes.dataitems import ImageDataItem, TableDataItem


class PersistenceMap(ImageDataItem):
    _name_template = r'PERSISTENCE_MAP'
    _title_template = "persistence map"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _description_template = "Persistence map"
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PERSISTENCE_MAP'

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }



class FluxCalTable(TableDataItem): # FixMe this should be a table, right?
    _name_template = r'FLUXCAL_TAB'
    _title_template = "flux table"
    _description_template = "Conversion between instrumental and physical flux units"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG'}

    _schema = {
        'PRIMARY': None,
        'TABLE': Table,
    }


class PinholeTable(TableDataItem):
    _title_template = "pinhole table"
    _name_template = r'PINHOLE_TABLE'
    _description_template = "Table of pinhole locations"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PINHOLE_TABLE'

    _schema = {
        'PRIMARY': None,
        'TABLE': Table,
    }


class AtmProfile(TableDataItem):
    _name_template = r'ATM_PROFILE'
    _title_template = "atmosphere profile"
    _description_template = ("Atmospheric profile containing height information on temperature, "
                             "pressure and molecular abundances")
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE

    _schema = {
        'PRIMARY': None,
        'TABLE': Table,
    }


class LsfKernel(TableDataItem):
    _name_template = r'LSF_KERNEL'
    _title_template = "line spread function kernel"
    _description_template = "Wavelength dependent model of the LSF"
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE


class FluxStdCatalog(TableDataItem):
    _name_template = r'FLUXSTD_CATALOG'
    _title_template = "catalog of standard stars"
    _description_template = "Catalog of standard stars"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = set()


class AtmLineCatalog(TableDataItem):
    _name_template = r'ATM_LINE_CAT'
    _title_template = "Line catalogue of atmospheric lines"
    _description_template = "Catalogue containing a line list of atmospheric molecular lines"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG'}


class LaserTable(TableDataItem):
    _name_template = r'LASER_TAB'
    _title_template = "Laser table"
    _description_template = "Table with laser lines"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL


class IfuTelluric(TableDataItem):
    _name_template = r'IFU_TELLURIC'
    _title_template = "Telluric correction"
    _description_template = "Transmission function for the telluric correction."
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}
