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

from pymetis.classes.dataitems import TableDataItem
from pymetis.classes.mixins import (BandSpecificMixin, BandLmMixin, BandNMixin, BandIfuMixin,
                                    CoronagraphSpecificMixin, CoronagraphCvcMixin, CoronagraphRavcMixin)


class SciThroughput(BandSpecificMixin, CoronagraphSpecificMixin, TableDataItem): # ToDo is this really a table?
    _name_template = r'{detector}_{cgrph}_SCI_THROUGHPUT'
    _title_template = "{detector} {cgrph} science throughput"
    _description_template = "{detector} {cgrph} ADI throughput curve"
    _oca_keywords = {'PRO.CATG', 'DRS.MASK'}


class LmCvcSciThroughput(BandLmMixin, CoronagraphCvcMixin, SciThroughput):
    pass


class NCvcSciThroughput(BandNMixin, CoronagraphCvcMixin, SciThroughput):
    pass


class IfuCvcSciThroughput(BandIfuMixin, CoronagraphCvcMixin, SciThroughput):
    pass


class LmRavcSciThroughput(BandLmMixin, CoronagraphRavcMixin, SciThroughput):
    pass


class NRavcSciThroughput(BandNMixin, CoronagraphRavcMixin, SciThroughput):
    pass


class IfuRavcSciThroughput(BandIfuMixin, CoronagraphRavcMixin, SciThroughput):
    pass

