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

from pymetis.engine.dataitems import detectors
from cpl.core import Image

from pymetis.instruments.metis.dataitems.raw import Raw
from pymetis.instruments.metis.mixins import BandIfuMixin, TargetStdMixin, TargetSciMixin, TargetSkyMixin
from pymetis.instruments.metis import keywords as kw


class IfuRaw(BandIfuMixin, Raw, abstract=True):
    _name_template = r'{band}_{target}_RAW'
    _title_template = r"{band} {target} raw"
    _description_template = r"{band} {target} raw image"
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _oca_keywords = frozenset({kw.DPR_CATG, kw.DPR_TECH, kw.DPR_TYPE, kw.INS_OPTI3_NAME,
                               kw.INS_OPTI9_NAME, kw.INS_OPTI10_NAME, kw.INS_OPTI11_NAME,
                               kw.DRS_IFU})
    _schema = detectors(Image, 4)


class IfuStdRaw(TargetStdMixin, IfuRaw):
    _description_template = "Raw spectra of flux standard star."
    _dpr = ('CALIB', 'IFU', 'STD')


class IfuSciRaw(TargetSciMixin, IfuRaw):
    _description_template = "IFU raw exposure of a science object."
    _dpr = ('SCIENCE', 'IFU', 'OBJECT')


class IfuSkyRaw(TargetSkyMixin, IfuRaw):
    """ The SKY leaf of the IFU raw template; a hand-written unrelated class with the literal
    name would be refused when `IfuRaw` is promoted with target='SKY'. """
    _description_template = "Blank sky image."
    _dpr = ('CALIB', 'IFU', 'SKY')
