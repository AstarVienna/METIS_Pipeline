import cpl
from cpl.core import Msg

from .base import MultiplePipelineInput, SinglePipelineInput


class RawInput(MultiplePipelineInput):
    _title: str = "raw"
    _group = cpl.ui.Frame.FrameGroup.RAW
    _required: bool = True


class MasterDarkInput(SinglePipelineInput):
    _title: str = "master dark"
    _tags: [str] = ["MASTER_DARK_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _required: bool = True


class LinearityInput(SinglePipelineInput):
    _title: str = "linearity map"
    _tags: [str] = ["LINEARITY_{det}"]
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _required: bool = True
