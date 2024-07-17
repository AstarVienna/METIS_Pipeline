import dataclasses
from abc import ABCMeta
from typing import Any

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl
from prototypes.input import PipelineInput


class RawImageProcessor(MetisRecipeImpl, metaclass=ABCMeta):
    class Input(PipelineInput):
        raw: cpl.ui.FrameSet = cpl.ui.FrameSet()

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self._detector_name = None

        def categorize_frame(self, frame):
            match frame.tag:
                case tag if tag in ["DARK_LM_RAW", "DARK_N_RAW", "DARK_IFU_RAW"]:
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw.append(frame)
                    Msg.debug(self.__class__.__name__,
                              f"Got raw frame: {frame.file}.")
                case _:
                    # If it is not recognized, let base classes handle it
                    super().categorize_frame(frame)

        def verify(self):
            if len(self.raw) == 0:
                raise cpl.core.DataNotFoundError("No raw frames found in the frameset.")

            detectors = []

            for frame in self.raw:
                header = cpl.core.PropertyList.load(frame.file, 0)
                det = header['ESO DPR TECH'].value
                try:
                    detectors.append({
                        'IMAGE,LM': '2RG',
                        'IMAGE,N': 'GEO',
                        'IFU': 'IFU',
                    }[det])
                except KeyError as e:
                    raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e

            # Check if all the raws have the same detector, if not, we have a problem
            if len(unique := list(set(detectors))) == 1:
                self._detector_name = unique[0]
            else:
                raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")

    def load_input_images(self) -> cpl.core.ImageList:
        """ Always load a set of raw images """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(self.input.raw):
            Msg.info(self.__class__.__name__, f"Processing input frame #{idx}: {frame.file!r}...")

            # Append the loaded image to an image list
            Msg.debug(self.__class__.__name__, f"Loading input image {frame.file}")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output

    @property
    def detector_name(self) -> str:
        return self.input._detector_name
