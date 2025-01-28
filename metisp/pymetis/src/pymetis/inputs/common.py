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

import re
import cpl
from typing import Pattern

from .single import SinglePipelineInput
from .multiple import MultiplePipelineInput

"""
This file contains various ready-to-use `PipelineInput` classes.
You should never derive directly from `PipelineInput`, but rather from

 -  `SinglePipelineInput` (for `Input` classes with a single Frame)
 -  `MultiplePipelineInput` (for `Input` classes with a FrameSet)

You should override class attributes:

 -  `_title`
    The short description of an `Input` class (this is only used for log output).
 -  `_group`
    The `cpl.ui.Frame.FrameGroup` property that is required by PyEsoRex and also by CPL.
 -  `_tags`
    The list of tags that are accepted by this input. Note that in some classes it is not defined yet, but definition
    is deferred to further children (but has to be defined *somewhere*). Tags can contain format template placeholders,
    which will be filled from **kwargs in the __init__ method, to allow various detectors or bands.
 -  `_required`
    A boolean telling the recipe if this input is required or not. Default is True, so it is enough to say
    `_required = False` for optional inputs.
"""


class RawInput(MultiplePipelineInput):
    _title: str = "raw"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW


class MasterDarkInput(SinglePipelineInput):
    _title: str = "master dark"
    _tags: Pattern = re.compile(r"MASTER_DARK_(?P<detector>2RG|GEO|IFU)")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class MasterFlatInput(SinglePipelineInput):
    _title: str = "master flat"
    _tags: Pattern = re.compile(r"MASTER_IMG_FLAT_LAMP_(?P<detector>2RG|GEO|IFU)")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class LinearityInput(SinglePipelineInput):
    _title: str = "linearity map"
    _tags: Pattern = re.compile(r"LINEARITY_(?P<detector>2RG|GEO|IFU)")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class BadpixMapInput(SinglePipelineInput):
    _title: str = "bad pixel map"
    _tags: Pattern = re.compile(r"BADPIX_MAP_(?P<detector>2RG|GEO|IFU)")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class PersistenceMapInput(SinglePipelineInput):
    _title: str = "persistence map"
    _tags: Pattern = re.compile(r"PERSISTENCE_MAP")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _required: bool = False     # Persistence maps are usually optional (but this can be overridden)


class GainMapInput(SinglePipelineInput):
    _title: str = "gain map"
    _tags: Pattern = re.compile(r"GAIN_MAP_(?P<detector>2RG|GEO|IFU)")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class DistortionTableInput(SinglePipelineInput):
    _title: str = "distortion table"
    _tags: Pattern = re.compile(r"IFU_DISTORTION_TABLE")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class WavecalInput(SinglePipelineInput):
    _title: str = "wavelength calibration"
    _tags: Pattern = re.compile(r"IFU_WAVECAL")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class PinholeTableInput(SinglePipelineInput):
    _title: str = "pinhole table"
    _tags: Pattern = re.compile(r"PINHOLE_TABLE")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
