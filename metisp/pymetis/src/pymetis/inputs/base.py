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

from abc import abstractmethod
import re
from typing import Pattern, Any

import cpl

from cpl.core import Msg


class PipelineInput:
    """
    The PipelineInput class describes an atomic part of an InputSet,
    either a single file (master dark, persistence map, etc.)
    or a set of structurally identical files with a common tag (e.g. sets of raw images).
    Methods for loading the files, validating the contents and hooks for class-specific checks are provided.
    """
    _title: str = None                      # No universal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _tags: Pattern = None                   # No universal tags are provided
    _group: cpl.ui.Frame.FrameGroup = None  # No sensible default, must be provided explicitly
    _detector: str | None = None            # Not specific to a detector until determined otherwise

    @property
    def title(self):
        return self._title

    @property
    def tags(self):
        return self._tags

    @property
    def required(self):
        return self._required

    @property
    def group(self):
        return self._group

    @property
    def detector(self):
        return self._detector

    def __init__(self,
                 *,
                 title: str = None,
                 tags: Pattern = None,
                 required: bool = None,
                 group: cpl.ui.Frame.FrameGroup = None,
                 **kwargs):
        # First override the title, if provided in the constructor
        if title is not None:
            self._title = title
            Msg.debug(self.__class__.__qualname__, f"Overriding `title` to {self.title}")

        # Check if it is defined
        if self.title is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        # First override the tags if provided in the ctor
        if tags is not None:
            self._tags = tags
            Msg.debug(self.__class__.__qualname__, f"Overriding `tags` to {self.tags}")

        # Check if tags are defined...
        if not self.tags:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tag pattern")

        # ...and that they are a re pattern
        if not isinstance(self.tags, re.Pattern):
            raise TypeError(f"PipelineInput `tags` must be a `re.Pattern`, got '{self.tags}'")

        # Override `required` if requested
        if required is not None:
            self._required = required
            Msg.debug(self.__class__.__qualname__, f"Overriding `required` to {self.required}")

        if group is not None:
            self._group = group
            Msg.debug(self.__class__.__qualname__, f"Overriding `group` to {self.group}")

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
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}Tag: {self.tags}")

    def as_dict(self) -> dict[str, Any]:
        return {
            'title': self.title,
            'tags': self.tags,
            'required': self.required,
            'group': self._group.name,
        }

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
                Msg.warning(self.__class__.__qualname__, f"No detector (ESO DPR TECH) set!")

        # Check if all the raws have the same detector, if not, we have a problem
        if (detector_count := len(unique := list(set(detectors)))) == 1:
            self._detector = unique[0]
            Msg.debug(self.__class__.__qualname__,
                      f"Detector determined: {self.detector}")
        elif detector_count == 0:
            Msg.warning(self.__class__.__qualname__,
                        f"No detectors specified (this is probably fine in skeleton stage)")
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