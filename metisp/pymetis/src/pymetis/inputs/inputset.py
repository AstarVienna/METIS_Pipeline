"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

from abc import ABCMeta

import cpl
from cpl.core import Msg

from pymetis.inputs.base import PipelineInput


class PipelineInputSet(metaclass=ABCMeta):
    """
        The `PipelineInput` class is a singleton utility class for a recipe.
        It reads and filters the input FrameSet, categorizes the frames by their metadata,
        and finally stores them in its own attributes for further use.
        It also provides verification mechanisms and methods
        for extraction of additional information from the frames.

        Every `RecipeImpl` should have exactly one `InputSet` class (though possibly shared by more recipes though).
        Currently, we define them as internal classes of the corresponding `RecipeImpl`,
        but in Python it does not really matter much, they can be instatiated or derived from from the outside too.
    """

    inputs: [PipelineInput] = []
    detector: str = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        """ Filter the input frameset, capture frames that match criteria and assign them to own attributes. """
        if not self.inputs:
            raise NotImplementedError(f"PipelineInput must define at least one input.")

        self.print_debug()

    def verify(self) -> None:
        Msg.debug(self.__class__.__qualname__, f"Verifying the inputset {self.inputs}")

        for inp in self.inputs:
            inp.verify()

        self.verify_detectors()

    def verify_detectors(self) -> None:
        """
        Verify that the provided SOF contains frames from only a single detector.
        Some Inputs have None if they are not specific to a detector.
        """
        detectors = list(set([inp.detector for inp in self.inputs]) - {None})
        if (detector_count := len(detectors)) != 1:
            Msg.debug(self.__class__.__qualname__, f"No detector could be identified from the SOF")
        elif detector_count == 1:
            self.detector = detectors[0]
        else:
            raise ValueError(f"More than one detector found in inputset: {detectors}")

    def print_debug(self, *, offset: int = 0) -> None:
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset} -- Detailed class info ---")
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}{len(self.inputs)} inputs:")
        Msg.debug(self.__class__.__qualname__, str(self.inputs))

        for inp in self.inputs:
            inp.print_debug(offset=offset + 4)
