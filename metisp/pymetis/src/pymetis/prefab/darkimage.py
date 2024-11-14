from abc import ABC

import cpl.ui

from pymetis.prefab.rawimage import RawImageProcessor


class DarkImageProcessor(RawImageProcessor, ABC):
    """
    DarkImageProcessor is a subclass of RawImageProcessor that:

     1. takes a set of raw images to combine
     2. requires a single `master_dark` frame, that will be subtracted from every raw image
     3. combines the raws into a single product

    Provides methods for loading and verification of the dark frame,
    warns if multiple master darks are provided, etc.
    """
    class InputSet(RawImageProcessor.InputSet):
        """
        A DarkImageProcessor's Input is just a raw image processor input with a master dark frame.
        """
        MasterDarkInput: type = None

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.master_dark = self.MasterDarkInput(frameset, det=self.detector)
            self.inputs += [self.master_dark]
