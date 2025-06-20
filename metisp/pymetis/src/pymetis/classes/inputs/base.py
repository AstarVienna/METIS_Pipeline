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

import re
from abc import abstractmethod
from typing import Pattern, Any, Optional

import cpl

from cpl.core import Msg


class PipelineInput:
    """
    This class encapsulates a single logical input to a recipe:
    - either a single file, or a line in the SOF (see SinglePipelineInput)
    - or a set of equivalent files (see MultiplePipelineInput)
    """
    _title: str = None                      # No universal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _tags: Pattern = None                   # No universal tags are provided
    _group: cpl.ui.Frame.FrameGroup = None  # No sensible default; must be provided explicitly
    _detector: Optional[str] = None         # Not specific to a detector until determined otherwise
    _description: Optional[str] = None      # Description for man page

    _multiplicity: str = '<undefined>'

    @classmethod
    def title(cls) -> str:
        return cls._title

    @classmethod
    def tags(cls) -> Pattern:
        """
        Return the tags for this pipeline input.
        Override the method if it should depend on some parameter.
        """
        return cls._tags

    @classmethod
    def required(cls) -> bool:
        return cls._required

    @classmethod
    def description(cls) -> str:
        return cls._description

    @property
    def group(self):
        return self._group

    @property
    def detector(self) -> str:
        return self._detector

    @detector.setter
    def detector(self, value):
        self._detector = value

    def __init__(self):
        # Check if it is defined
        if self.title() is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        # Check if tags are defined...
        if not self.tags():
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tag pattern")

        # ...and that they are a `re` pattern
        if not isinstance(self.tags(), re.Pattern):
            raise TypeError(f"PipelineInput `tags` must be a `re.Pattern`, got '{self.tags()}'")

        # Check is frame_group is defined (if not, this gives rise to strange errors deep within CPL
        # that you really do not want to deal with)
        if not self.group:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined group!")

        # A list of matched groups from `tags`. Acquisition differs
        # between Single and Multiple, so we just declare it here.
        self.tag_parameters: dict[str, str] = {}

    @abstractmethod
    def validate(self) -> None:
        """
        Verify that the input has all the required frames and that they are valid themselves.
        There is no default logic, implementation is fully deferred to derived classes.
        """

    def print_debug(self, *, offset: int = 0) -> None:
        """
        Print a short description of the tags with a small offset (n spaces).
        """
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}Tag: {self.tags()}")

    def as_dict(self) -> dict[str, Any]:
        return {
            'title': self.title,
            'tags': self.tags(),
            'required': self.required,
            'group': self._group.name,
        }

    @classmethod
    def description_line(cls) -> str:
        return (f"    {cls._pretty_tags():<60} [{cls._multiplicity}]"
                f"{' (optional)' if not cls._required else '           '} "
                f"{cls._description}")

    def _verify_same_detector_from_header(self) -> None:
        """
        Verification for headers, currently disabled
        """
        detectors = []
        for frame in self.frameset:
            header = cpl.core.PropertyList.load(frame.file, 0)
            try:
                det = header['ESO DPR TECH'].value
                try:
                    detectors.append({
                                         'IMAGE,LM': '2RG',
                                         'IMAGE,N': 'GEO',
                                         'IFU': 'IFU',
                                     }[det])
                except KeyError as e:
                    raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e
            except KeyError:
                Msg.warning(self.__class__.__qualname__, "No detector (ESO DPR TECH) set!")

        # Check if all the raws have the same detector, if not, we have a problem
        if (detector_count := len(unique := list(set(detectors)))) == 1:
            self._detector = unique[0]
            Msg.debug(self.__class__.__qualname__,
                      f"Detector determined: {self.detector}")
        elif detector_count == 0:
            Msg.warning(self.__class__.__qualname__,
                        "No detectors specified (this is probably fine in skeleton stage)")
        else:
            # raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")
            Msg.warning(self.__class__.__qualname__,
                        f"Darks from more than one detector found: {unique}!")

    @abstractmethod
    def valid_frames(self) -> cpl.ui.FrameSet:
        """
        Return a FrameSet containing all valid, used frames.
        This is abstract as it differes significantly for Single and Multiple Inputs.
        """
        pass

    @classmethod
    def _pretty_tags(cls) -> str:
        """ Helper method to print `re.Pattern`s in man-page: remove named capture groups' names. """
        return cls.tags().pattern
        # return re.sub(r"\?P<\w+>", "", cls.tags().pattern)
