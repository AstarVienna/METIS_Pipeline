from abc import ABC

import cpl

from pymetis.classes.dataitems.dataitem import DataItem


class MasterDark(DataItem, ABC):
    _title: str = r"master dark"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Abstract base class for master darks. Please subclass."