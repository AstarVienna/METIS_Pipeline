import dataclasses
from abc import ABCMeta

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl


class RawImageProcessor(MetisRecipeImpl, metaclass=ABCMeta):
    class Input(MetisRecipeImpl.Input):
        raw: cpl.ui.FrameSet = cpl.ui.FrameSet()

        def categorize_frame(self, frame):
            match frame.tag:
                case tag if tag in ["DARK_LM_RAW", "DARK_N_RAW", "DARK_IFU_RAW"]:
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw.append(frame)
                    Msg.debug(self.__class__.__name__,
                              f"Got raw frame: {frame.file}.")
                case _:
                    Msg.warning(self.__class__.__name__,
                                f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.")

        def verify(self):
            if len(self.raw) == 0:
                raise cpl.core.DataNotFoundError("No raw frames found in the frameset.")

    def verify_input_frames(self) -> None:
        """ RawImageProcessor mixin wants to see a bunch of raw frames. """
        self.input.verify()

    def load_input_images(self) -> cpl.core.ImageList:
        """ Always load a set of raw images """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(self.input.raw):
            Msg.info(self.name, f"Processing input frame #{idx}: {frame.file!r}...")

            # Append the loaded image to an image list
            Msg.debug(self.name, f"Loading input image {frame.file}")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output
