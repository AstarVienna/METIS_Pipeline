import cpl
from cpl.core import Msg

from .base import MultiplePipelineInput, SinglePipelineInput


"""
This file contains various ready-to-use PipelineInput classes.
You should never derive directly from `PipelineInput`, but rather from

 -  `SinglePipelineInput` (for Input classes with a single Frame)
 -  `MultiplePipelineInput` (for Input classes with a FrameSet)
 
You should override class attributes:

 -  `_title`
    the short description of an Input class (this is used for log output)
 -  `_group`
    the `cpl.ui.Frame.FrameGroup` property that is required by PyEsoRex and also by CPL
 -  `_tags`
    the list of tags that are accepted by this input. Note that in some classes it is not defined yet, but definition
    is deferred to further children (but has to be defined *somewhere*). Tags can contain format template placeholders,
    which will be filled from **kwargs in the __init__ method, to allow various detectors or bands.
 -  `_required`
    a boolean telling the recipe if this input is required or not. Default is True, so it is enough to say
    `_required = False` for optional inputs.
"""


class RawInput(MultiplePipelineInput):
    _title: str = "raw"
    _group = cpl.ui.Frame.FrameGroup.RAW


class MasterDarkInput(SinglePipelineInput):
    _title: str = "master dark"
    _tags: [str] = ["MASTER_DARK_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class MasterFlatInput(SinglePipelineInput):
    _title: str = "master flat"
    _tags: [str] = ["MASTER_IMG_FLAT_LAMP_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class LinearityInput(SinglePipelineInput):
    _title: str = "linearity map"
    _tags: [str] = ["LINEARITY_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB


class PersistenceMapInput(SinglePipelineInput):
    _title: str = "persistence map"
    _tags: [str] = ["PERSISTENCE_MAP"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _required: bool = False     # Persistence maps are usually optional (can be overridden)


class GainMapInput(SinglePipelineInput):
    _title: str = "gain map"
    _tags = ["GAIN_MAP_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
