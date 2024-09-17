from abc import ABC

import cpl
from cpl.core import Msg

from prototypes.rawimage import RawImageProcessor
from prototypes.mixins import MasterDarkInputMixin


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    DarkImageProcessor is a subclass of RawImageProcessor that also requires a single `master_dark` frame.
    Provides methods for loading and verification of the dark frame, warns if multiple master darks are provided, etc.
    """
    class Input(MasterDarkInputMixin, RawImageProcessor.Input):
        """
        A DarkImageProcessor's Input is just a raw image processor input with a master dark frame mixin.
        """
