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
from pymetis.classes.mixins import BandSpecificMixin, BandIfuMixin, BandLmMixin


class SciCoadd(BandSpecificMixin, DataItem, abstract=True):
    _name_template = r'{band}_SCI_COADD'
    _title_template = r"{band} science co-added"
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_type = cpl.ui.Frame.FrameType.IMAGE
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}



class LmSciCoadd(BandLmMixin, SciCoadd):
    _description_template = "Coadded, mosaiced LM image"


class IfuSciCoadd(BandIfuMixin, SciCoadd):
    _description_template = ("Spectral cube of science object, a coadd of a number of"
                             "reduced IFU exposures covering a different spatial and wavelength ranges.")