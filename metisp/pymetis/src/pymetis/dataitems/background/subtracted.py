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
from cpl.core import Image as CplImage

from pymetis.classes.dataitems import ImageDataItem
from pymetis.classes.mixins import (TargetSpecificMixin, TargetSciMixin, TargetStdMixin,
                                    BandSpecificMixin, BandLmMixin, BandNMixin)


class BackgroundSubtracted(BandSpecificMixin, TargetSpecificMixin, ImageDataItem, abstract=True):
    _name_template = r'{band}_{target}_BKG_SUBTRACTED'
    _title_template = "{band} background-subtracted"
    _description_template = r"Thermal background subtracted images of science {band} {target} exposures."
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'} # maybe

    _schema = [CplImage]


class StdBackgroundSubtracted(TargetStdMixin, BackgroundSubtracted, abstract=True):
    pass


class LmStdBackgroundSubtracted(BandLmMixin, StdBackgroundSubtracted):
    pass


class NStdBackgroundSubtracted(BandNMixin, StdBackgroundSubtracted):
    pass


class SciBackgroundSubtracted(TargetSciMixin, BackgroundSubtracted, abstract=True):
    pass


class LmSciBackgroundSubtracted(BandLmMixin, SciBackgroundSubtracted):
    pass


class NSciBackgroundSubtracted(BandNMixin, SciBackgroundSubtracted):
    pass


