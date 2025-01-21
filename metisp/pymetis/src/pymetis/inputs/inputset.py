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
import copy
import functools
import operator
from abc import ABCMeta

import cpl
from cpl.core import Msg

from pymetis.inputs.base import PipelineInput


class PipelineInputSet(metaclass=ABCMeta):
    """
    The `PipelineInputSet` class is a utility class for a recipe.
    It reads and filters the input FrameSet, categorizes the frames by their metadata,
    and finally stores them in its own attributes for further use.
    It also provides verification mechanisms and methods
    for extraction of additional information from the frames.

    Every `RecipeImpl` should have exactly one `InputSet` class (possibly shared by multiple recipes).
    Currently, we define them as internal classes of the corresponding `RecipeImpl`,
    but in Python it does not really matter much and does not imply any particular relationship
    between the classes -- it is just a namespacing convention.
    """

    detector: str = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        """
            Filter the input frameset, capture frames that match criteria and assign them to own attributes.
            By default, there is nothing.
        """
        self.inputs = []

    def validate(self) -> None:
        Msg.debug(self.__class__.__qualname__, f"Validating the inputset {self.inputs}")

        if not self.inputs:
            raise NotImplementedError(f"PipelineInput must define at least one input.")

        self.print_debug()

        for inp in self.inputs:
            inp.validate()

            print(f"{self.__class__.__qualname__}::{inp.__class__.__qualname__} input tag parameters: {inp.tag_parameters}")

        #functools.reduce(operator.or_, [inp.parameter_tags for inp in self.inputs])

        self.validate_detectors()

    def validate_detectors(self) -> None:
        """
        Verify that the provided SOF contains frames from only a single detector.
        Some Inputs may have None if they are not specific to a detector.
        """
        detectors = list(set([inp.detector for inp in self.inputs]) - {None})
        if (detector_count := len(detectors)) == 0:
            Msg.warning(self.__class__.__qualname__, f"No detector could be identified from the SOF")
        elif detector_count == 1:
            self.detector = detectors[0]
            Msg.debug(self.__class__.__qualname__, f"Detector identified from the SOF: {self.detector}")
        else:
            raise ValueError(f"More than one detector found in inputset: {detectors}")

    def print_debug(self, *, offset: int = 0) -> None:
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset} -- Detailed class info ---")
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}{len(self.inputs)} inputs:")
        Msg.debug(self.__class__.__qualname__, str(self.inputs))

        for inp in self.inputs:
            inp.print_debug(offset=offset + 4)
