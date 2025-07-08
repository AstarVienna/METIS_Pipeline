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

import inspect
import re

from abc import ABC
from typing import Any, Callable, Optional

import cpl
from cpl.core import Msg

from pymetis.classes.inputs.base import PipelineInput
from pymetis.classes.mixins import TargetSpecificMixin, SourceSpecificMixin, BandSpecificMixin, DetectorSpecificMixin
from pymetis.classes.mixins.base import Parametrizable


class PipelineInputSet(Parametrizable, ABC):
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

    # Helper regex: remove final "Input" from the name of the class...
    cut_input = re.compile(r'Input$')
    # Helper regex: ...and then turn PascalCase to snake_case (to obtain the instance name from class name)
    make_snake = re.compile(r'(?<!^)(?=[A-Z])')
    # I.e. `MonsterCrunchInput` class instance will be named `monster_crunch`.

    def __init_subclass__(cls, abstract=False, **params):
        cls.__abstract = abstract

        if cls.__abstract:
            pass
        else:
            for name, input_class in cls.get_inputs():
                print(f"Found input class {cls.__qualname__} {input_class.__name__} "
                      f"({input_class.Item._name_template}), "
                      f"params are {cls.tag_parameters()}")

    def __init__(self, frameset: cpl.ui.FrameSet):
        """
        Filter the input frameset, capture frames that match criteria and assign them to your own attributes.
        By default, there is nothing: no inputs, no tag_parameters.
        This feels hacky but makes it much more comfortable as you do not need to define Inputs manually.
        """
        self.inputs: set[PipelineInput] = set()         # A set of all inputs for this InputSet.
        self.frameset: cpl.ui.FrameSet = frameset

        # Tag parameter matching this instance of InputSet. Might come from DataItem matches or hard-coded from mixins.
        self.tag_matches: dict[str, str] = {}

        # Now iterate over all defined Inputs, instantiate them and feed them the frameset to filter.
        for (name, input_class) in self.get_inputs():
            inp = input_class(frameset)
            # FixMe: very hacky for now: determine the name of the instance from the name of the class
            self.__setattr__(self.make_snake.sub('_', self.cut_input.sub('', name)).lower(), inp)
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
        Msg.debug(self.__class__.__qualname__,
                  f"Validating the inputset {self.inputs}")

        if len(self.inputs) == 0:
            raise NotImplementedError("PipelineInputSet must define at least one input.")

        for inp in self.inputs:
            inp.validate()

        for attr, klass in [
            ('detector', DetectorSpecificMixin),
            ('band', BandSpecificMixin),
            ('target', TargetSpecificMixin),
            ('source', SourceSpecificMixin),
        ]:
            self._validate_attr(lambda x: x.Item.tag_parameters()[attr] if issubclass(x.Item, klass) else None, attr)

    def _validate_attr(self, _func: Callable, attr: str) -> Optional[str]:
        """
        Helper method: validate the input attribute (detector, band, source or target).

        Return
            None, if the attribute cannot be identified (this usually is not an error if it is not defined).
            The attribute value, if the attribute only has the same value everywhere.
        Raise
            ValueError if the attribute has multiple different values.
        """
        Msg.debug(self.__class__.__qualname__, f"--- Validating the {attr} parameters ---")

        self.tag_matches[attr] = self.tag_parameters()[attr] if attr in self.tag_parameters() else None

        total = list(set([_func(inp) for inp in self.inputs]) - {None})

        for inp in self.inputs:
            value = _func(inp)
            det = "---" if value is None else value
            Msg.debug(self.__class__.__qualname__,
                      f"{attr:<15s} in {inp.__class__.__qualname__:<40} {det}")

        if (count := len(total)) == 0:
            # If there are no identifiable tag parameters, just emit a message
            Msg.debug(self.__class__.__qualname__,
                      f"No {attr} could be identified from the SOF")
        elif count == 1:
            # If there is exactly one unique value, the input is consistent, so set it as an InputSet tag parameter
            result = total[0]
            Msg.debug(self.__class__.__qualname__,
                      f"Correctly identified {attr} from the SOF: {result}")
            self.tag_matches[attr] = result
        else:
            # If there is more than one unique value, the input is inconsistent, raise an exception
            raise ValueError(f"Data from more than one {attr} found in inputset: {total}!")

    def print_debug(self, *, offset: int = 0) -> None:
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}--- Detailed class info ---")
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}{len(self.inputs)} inputs:")

        for inp in self.inputs:
            Msg.debug(self.__class__.__qualname__, f"   {inp.Item.__qualname__:<30s} {inp.item()}")

    def as_dict(self) -> dict[str, Any]:
        """
        Return a dict representation of the input patterns.
        """
        return {
            inp.Item.name(): inp.as_dict()
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
