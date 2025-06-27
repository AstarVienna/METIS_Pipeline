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

import functools
import inspect
import operator
import re

from abc import ABCMeta
from typing import Any

import cpl
from cpl.core import Msg

from pymetis.classes.inputs.base import PipelineInput


class PipelineInputSet(metaclass=ABCMeta):
    """
    The `PipelineInputSet` class is a utility class for a recipe dealing with the input data.
    It reads and filters the input FrameSet, categorizes the frames by their metadata,
    and finally stores them in its own attributes for further use.
    It also provides verification mechanisms and methods
    for extraction of additional information from the frames.

    Every `RecipeImpl` should have exactly one `InputSet` class
    (possibly but not necessarily shared by multiple recipes).
    Currently, we define them as internal classes of the corresponding `RecipeImpl`,
    but in Python it does not really matter much and does not imply any particular relationship
    between the classes -- it is just a namespacing convention.
    """

    detector: str = NotImplemented

    def __init__(self, frameset: cpl.ui.FrameSet):
        """
            Filter the input frameset, capture frames that match criteria and assign them to your own attributes.
            By default, there is nothing: no inputs, no tag_parameters.
            This feels hacky but makes it much more comfortable as you do not need to define Inputs manually.
        """
        self.inputs: set[PipelineInput] = set()         # A set of all inputs for this InputSet.
        self.tag_parameters: dict[str, str] = {}        # A dict of all tunable parameters determined from tags
        self.frameset: cpl.ui.FrameSet = frameset

        # Regex: remove final "Input" from the name of the class...
        cut_input = re.compile(r'Input$')
        # Regex: ...and then turn PascalCase to snake_case (to obtain the instance name from class name)
        make_snake = re.compile(r'(?<!^)(?=[A-Z])')

        # Now iterate over all defined Inputs, instantiate them and feed them the frameset to filter.
        for (name, input_type) in self.get_inputs():
            inp = input_type(frameset)
            # FixMe: very hacky for now: determine the name of the instance from the name of the class
            self.__setattr__(make_snake.sub('_', cut_input.sub('', name)).lower(), inp)
            # Add to the set of inputs (for easy iteration over all inputs)
            self.inputs |= {inp}

    @classmethod
    def get_inputs(cls):
        return inspect.getmembers(cls, lambda x: inspect.isclass(x) and issubclass(x, PipelineInput))

    def validate(self) -> None:
        """
        Validate the inputset:
            - see that all inputs are loaded
                - and that they are themselves valid
                - and that they are processing compatible data (same detector, etc.)
            - parse the tag parameters
                - and assign their values as attributes of the inputset
        """
        Msg.debug(self.__class__.__qualname__, f"Validating the inputset {self.inputs}")

        if len(self.inputs) == 0:
            raise NotImplementedError("PipelineInputSet must define at least one input.")

        for inp in self.inputs:
            inp.validate()

        self.validate_detectors()
        self.tag_parameters = functools.reduce(operator.or_, [inp.tag_parameters for inp in self.inputs], {})

        # For every parsed tag parameter, set the corresponding attribute
        for key, value in self.tag_parameters.items():
            Msg.info(self.__class__.__qualname__, f"Setting PipelineInputSet tag parameter '{key}' = '{value}'")
            self.__setattr__(key, value)

    def validate_detectors(self) -> None:
        """
        Verify that the provided SOF only contains frames from a single detector.
        Some Inputs may return `None` if they are not specific to a detector.
        """
        Msg.debug(self.__class__.__qualname__, "--- Validating the detector parameters ---")
        detectors = list(set([inp.Item.detector() for inp in self.inputs]) - {None})

        for inp in self.inputs:
            det = "---" if inp.Item.detector() is None else inp.Item.detector()
            Msg.debug(self.__class__.__qualname__,
                      f"Detector in {inp.__class__.__qualname__:<50} {det}")

        if (detector_count := len(detectors)) == 0:
            Msg.debug(self.__class__.__qualname__,
                      "No detector could be identified from the SOF")
        elif detector_count == 1:
            self.detector = detectors[0]
            Msg.debug(self.__class__.__qualname__,
                      f"Detector correctly identified from the SOF: {self.detector}")
        else:
            raise ValueError(f"Data from more than one detector found in inputset: {detectors}!")

    def print_debug(self, *, offset: int = 0) -> None:
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}--- Detailed class info ---")
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}{len(self.inputs)} inputs:")

        for inp in self.inputs:
            Msg.debug(self.__class__.__qualname__, f"   {inp.Item.__qualname__}")

    def as_dict(self) -> dict[str, Any]:
        """
        Return a dict representation of the input patterns.
        """
        return {
            inp.Item.name: inp.as_dict()
            for inp in self.inputs
        }

    @property
    def valid_frames(self) -> cpl.ui.FrameSet:
        frameset = cpl.ui.FrameSet()

        for inp in self.inputs:
            frames = inp.valid_frames()
            for frame in frames:
                frameset.append(frame)

        return frameset

    @property
    def used_frames(self) -> cpl.ui.FrameSet:
        """
        Return the frames that actually affect the output anyhow (if a frame is not listed here, the output without
        that frame should be identical

        - [HB]: also includes frames that do not contribute any pixel data,
                for instance, discarded outliers (without them a different frame might be an outlier)
        # FixMe: Currently this only ensures that frames are loaded, not actually used!
        """
        return self.valid_frames
