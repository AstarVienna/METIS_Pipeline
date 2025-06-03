from abc import ABC

import cpl

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin


class MasterDark(DataItem, ABC):
    _title: str = r"master dark"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Abstract base class for master darks. Please subclass."


class MasterDark2rg(Detector2rgMixin, MasterDark):
    pass


class MasterDarkGeo(DetectorGeoMixin, MasterDark):
    pass


class MasterDarkIfu(DetectorIfuMixin, MasterDark):
    pass