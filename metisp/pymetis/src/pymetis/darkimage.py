from abc import ABC

import cpl
from cpl.core import Msg

from pymetis.rawimage import RawImageProcessor
from pymetis.mixins import MasterDarkInputMixin


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    DarkImageProcessor is a subclass of RawImageProcessor that:

     1. takes a set of raw images to combine
     2. requires a single `master_dark` frame, that will be subtracted from every raw image
     3. combines the raws into a single product

    Provides methods for loading and verification of the dark frame,
    warns if multiple master darks are provided, etc.
    """
    class Input(MasterDarkInputMixin, RawImageProcessor.Input):
        """
        A DarkImageProcessor's Input is just a raw image processor input with a master dark frame mixin.
        No further actions or methods are required.
        """
