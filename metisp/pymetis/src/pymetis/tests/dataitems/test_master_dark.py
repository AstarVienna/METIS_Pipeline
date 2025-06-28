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

from pymetis.classes.dataitems.masterdark.masterdark import MasterDark2rg, MasterDarkGeo, MasterDarkIfu
from pymetis.classes.dataitems.masterflat import MasterFlat2rg, MasterFlatGeo, MasterFlatIfu
from pymetis.tests.classes.dataitem import DataItemTest


class TestMasterDark2rg(DataItemTest):
    Item = MasterDark2rg


class TestMasterDarkGeo(DataItemTest):
    Item = MasterDarkGeo


class TestMasterDarkIfu(DataItemTest):
    Item = MasterDarkIfu


class TestMasterFlat2rg(DataItemTest):
    Item = MasterFlat2rg


class TestMasterFlatGeo(DataItemTest):
    Item = MasterFlatGeo


class TestMasterFlatIfu(DataItemTest):
    Item = MasterFlatIfu