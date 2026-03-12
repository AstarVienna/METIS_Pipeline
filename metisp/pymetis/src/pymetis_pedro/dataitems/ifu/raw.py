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
from cpl.core import Image

from pymetis.dataitems.raw import Raw
from pymetis.classes.mixins import BandIfuMixin, TargetSpecificMixin, TargetStdMixin, TargetSciMixin


class IfuRaw(BandIfuMixin, TargetSpecificMixin, Raw, abstract=True):
    _name_template = r'{band}_{target}_RAW'
    _title_template = r"{band} {target} raw"
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _oca_keywords = {"DPR.CATG", "DPR.TECH", "DPR.TYPE", "INS.OPTI3.NAME",
                     "INS.OPTI9.NAME", "INS.OPTI10.NAME", "INS.OPTI11.NAME",
                     "DRS.IFU"}
    _schema = {
        'PRIMARY': None,
    } | {
        fr'DET{det:1d}.DATA': Image for det in range(1, 5)
    }



class IfuStdRaw(TargetStdMixin, IfuRaw):
    _description_template = "Raw spectra of flux standard star."


class IfuSciRaw(TargetSciMixin, IfuRaw):
    _description_template = "IFU raw exposure of a science object."


class IfuSkyRaw(BandIfuMixin, Raw):
    _name_template = r'IFU_SKY_RAW'
    _title_template = r"IFU sky raw"
    _frame_level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _oca_keywords = {"DPR.CATG", "DPR.TECH", "DPR.TYPE", "INS.OPTI3.NAME",
                     "INS.OPTI9.NAME", "INS.OPTI10.NAME", "INS.OPTI11.NAME",
                     "DRS.IFU"}
    _description_template = "Blank sky image."
