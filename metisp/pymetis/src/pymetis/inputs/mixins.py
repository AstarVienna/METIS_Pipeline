import cpl.ui

from pymetis.inputs import PersistenceMapInput, PipelineInputSet


class Detector2rgMixin:
    detector: str = '2RG'
    band: str = 'LM'

class DetectorGeoMixin:
    detector: str = 'GEO'
    band: str = 'N'

class DetectorIfuMixin:
    detector: str = 'IFU'

class TargetSciMixin:
    target: str = 'SCI'

class TargetStdMixin:
    target: str = 'STD'



class PersistenceInputSetMixin(PipelineInputSet):
    def __init__(self, frameset: cpl.ui.FrameSet):
        self.persistence_map = PersistenceMapInput(frameset)
        super().__init__(frameset)
        self.inputs += [self.persistence_map]