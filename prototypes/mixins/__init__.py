# Basic reusable mixin classes
from .masterdark import MasterDarkInputMixin
from .masterflat import MasterFlatInputMixin
from .linearity import LinearityInputMixin
from .badpixmap import BadpixMapInputMixin
from .persistence import PersistenceInputMixin
from .gainmap import GainMapInputMixin, GainMap2rgInputMixin

# Mixins that set the detector name (and any related data if needed)
from .detectors import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin