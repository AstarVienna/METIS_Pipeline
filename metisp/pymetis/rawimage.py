import dataclasses
from abc import ABC
from typing import Any, Literal

import cpl
from cpl.core import Msg

from prototypes.base import MetisRecipeImpl
from prototypes.input import PipelineInput


class RawImageProcessor(MetisRecipeImpl, ABC):
    """
    RawImageProcessor is a recipe implementation that takes a bunch of raw frames,
    categorizes them according to their properties and outputs and performs a sanity check or two.
    """

    class Input(PipelineInput):
        """
        Generic Input class for RawImageProcessor.
        Must define `tags_raw`, the set of tags which match files that should be processed.
        """
        tags_raw: [str] = []

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.raw: cpl.ui.FrameSet = cpl.ui.FrameSet()
            self._detector_name = None

            super().__init__(frameset)

            if not self.tags_raw:
                raise NotImplementedError("RawImageProcessor Input must define `tags_raw`.")

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case tag if tag in self.tags_raw:
                    frame.group = cpl.ui.Frame.FrameGroup.RAW
                    self.raw.append(frame)
                    Msg.debug(self.__class__.__qualname__, f"Got raw frame: {frame.file}.")
                case _:
                    # If `frame.tag` was not recognized, percolate up and let base classes handle it
                    super().categorize_frame(frame)

        def verify(self) -> None:
            self._verify_frameset_not_empty(self.raw, "raw frames")
            self._verify_same_detector()

        def _verify_same_detector(self) -> None:
            """
            Verify whether all the raw frames originate from the same detector.

            Raises
            ------
            KeyError
                If the detector name is not a valid detector name
            ValueError
                If dark frames from more than one detector are found

            Returns
            -------
            None:
                None on success
            """
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

    def load_raw_images(self) -> cpl.core.ImageList:
        """
        Always load a set of raw images, as determined by the tags.
        Chi-Hung has warned Martin that this is unnecessary and fills the memory quickly,
        but if we are to use CPL functions, Martin does not see a way around it.
        """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(self.input.raw):
            Msg.info(self.__class__.__qualname__, f"Processing input frame #{idx}: {frame.file!r}...")

            # Append the loaded image to an image list
            Msg.debug(self.__class__.__qualname__, f"Loading input image {frame.file}")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output

    @classmethod
    def combine_images(cls,
                       images: cpl.core.ImageList,
                       method: Literal['add'] | Literal['average'] | Literal['median']):
        """
        Basic helper method to combine images using one of `add`, `average` or `median`.
        Probably not a universal panacea, but it recurs often enough to warrant being here.
        """
        Msg.info(cls.__qualname__, f"Combining images using method {method!r}")
        combined_image = None
        match method:
            case "add":
                for idx, image in enumerate(images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = images.collapse_create()
            case "median":
                combined_image = images.collapse_median_create()
            case _:
                Msg.error(cls.__qualname__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")
                raise ValueError(f"Unknown stacking method {method!r}")

        return combined_image

    @property
    def detector_name(self) -> str:
        return self.input._detector_name

