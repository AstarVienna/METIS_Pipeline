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

from pymetis.instruments.metis.dataitems.raw import Raw
from pymetis.instruments.metis.mixins import (BandLmMixin, BandNMixin,
                                    TargetStdMixin, TargetSciMixin, TargetSkyMixin)
from pymetis.instruments.metis import keywords as kw


class ImageRaw(Raw, abstract=True):
    """
    Abstract intermediate class for image raws.
    """
    _name_template = r'{band}_IMAGE_{target}_RAW'
    _title_template = "{band} image {target} raw"
    _description_template = "Raw exposure of a {target} in the {band} image mode."
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _oca_keywords = frozenset({kw.DPR_CATG, kw.DPR_TECH, kw.DPR_TYPE, kw.INS_OPTI3_NAME,
                               kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.DRS_FILTER})


class LmImageRaw(BandLmMixin, ImageRaw, abstract=True):
    pass


class LmImageStdRaw(TargetStdMixin, LmImageRaw):
    _dpr = ('CALIB', 'IMAGE,{band}', 'STD')


class LmImageSciRaw(TargetSciMixin, LmImageRaw):
    _dpr = ('SCIENCE', 'IMAGE,{band}', 'OBJECT')


class LmImageSkyRaw(TargetSkyMixin, LmImageRaw):
    _dpr = (None, 'IMAGE,{band}', 'SKY')          # a sky frame is CALIB or SCIENCE, as the workflow rule had it


class NImageRaw(BandNMixin, ImageRaw, abstract=True):
    pass


class NImageStdRaw(TargetStdMixin, NImageRaw):
    _dpr = ('CALIB', 'IMAGE,{band}', 'STD')


class NImageSciRaw(TargetSciMixin, NImageRaw):
    _dpr = ('SCIENCE', 'IMAGE,{band}', 'OBJECT')
