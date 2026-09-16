"""
This file is part of an A* Pipeline.
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

from abc import ABC, abstractmethod
from typing import Any, final, Union, ClassVar

import cpl
from cpl.core import Msg

from pymetis.engine.core.functions.format import partial_format
from pymetis.engine.core.functions.frameset import preprocess_frameset
from pymetis.engine.dataitems.dataitem import DataItem


class PipelineInput(ABC):
    """
    This class encapsulates a single logical input to a recipe:
    - either a single file, or a line in the SOF (see SinglePipelineInput)
    - or a set of equivalent files (see MultiplePipelineInput)

    It is a relatively thin wrapper over the inner `Item`.
    """
    Item: ClassVar[type[DataItem]] = None   # No universal data item inside
    _title: str = None                      # No universal title makes sense
    _required: bool = True                  # By default, inputs are required to be present

    _multiplicity: ClassVar[str] = None     # Multiplicity of the input, '1' or 'N'

    # The role of the frames in *this* recipe, stamped on them when loaded: RAW for the
    # frames being reduced (CPL DFS inherits the product header from the first of them),
    # CALIB for everything applied to them. This is a property of the input, not of the
    # item: a science product is a calibration to one recipe and the raw material of the
    # next. The item's own frame group describes its origin (instrument data, calibration,
    # pipeline product) and is used when it is saved.
    _group: ClassVar[cpl.ui.Frame.FrameGroup] = cpl.ui.Frame.FrameGroup.CALIB

    def load_frameset(self, frameset: cpl.ui.FrameSet) -> None:
        """
        Load the associated frames.

        Fills the internal buffer with the contents of the frames (not the actual data yet).
        """
        self._load_frameset_specific(frameset)
        self.set_cpl_attributes()

    @abstractmethod
    def _load_frameset_specific(self, frameset: cpl.ui.FrameSet) -> None:
        """
        Load the associated frames. Implementation differs between derived classes.
        """
        pass

    @abstractmethod
    def set_cpl_attributes(self):
        """
        Stamp the CPL attributes on the loaded frames: the group is this input's role
        (`_group`), level and type come from the item. CPL DFS reads them when the
        product header is built, so this must run before any product is saved.
        """

    @classmethod
    def required(cls) -> bool:
        """
        Returns whether this pipeline input is required. Used during validation.
        """
        return cls._required

    @classmethod
    def multiplicity(cls) -> str:
        """
        Returns the multiplicity of the input ('1' or 'N')
        """
        return cls._multiplicity

    def __init__(self, frameset: cpl.ui.FrameSet):
        """
        Verify that all required class attributes are defined
        and promote to the most specialized derived class
        depending on the input frameset.
        """
        assert self.Item is not None, \
            f"Pipeline input {self.__class__.__qualname__} has no defined data item"

        assert self.Item.name() is not None, \
            f"Data item {self.Item.__qualname__} has no defined name"

        assert self.Item.frame_type() is not None, \
            f"Data item {self.Item.__qualname__} has no defined frame type"

        assert self.Item.frame_level() is not None, \
            f"Data item {self.Item.__qualname__} has no defined frame level"

        assert self.Item.frame_group() is not None, \
            f"Data item {self.Item.__qualname__} has no defined frame group"

        # Match all frames that can be processed by this PipelineInput.
        Msg.debug(self.__class__.__qualname__,
                  f"Initializing an input {self.Item.name()}")

        matches: dict[str, tuple[type[DataItem], cpl.ui.FrameSet]] = {}
        for tag, frames in preprocess_frameset(frameset).items():
            cls = DataItem.find(tag)
            if cls is None:
                Msg.warning(self.__class__.__qualname__,
                            f"Found a frame with tag '{tag}', which is not a registered data item. Ignoring.")
            elif issubclass(cls, self.Item):
                matches[tag] = (cls, frames)
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"Found {cls.__name__} with tag {tag}, "
                          f"which is not a {self.Item.__qualname__} ({self.Item.name()})")

        # Frames of several different data items (e.g. two detectors) can never
        # belong to one input; loading them in turn would silently keep the last.
        if len(matches) > 1:
            raise cpl.core.IllegalInputError(
                f"{self.__class__.__qualname__}: frames of several different data items match "
                f"the input {self.Item.name()}: {sorted(matches)}. "
                f"The set of frames must provide exactly one of them.")

        for tag, (cls, frames) in matches.items():
            if cls is not self.Item:
                # Promote this instance to the more specialized class found in the frames.
                Msg.debug(self.__class__.__qualname__,
                          f"Found a specialized class {cls.__qualname__} for {tag}, "
                          f"promoting this {self.Item.__qualname__}")
                self.Item = cls
            self.load_frameset(frames)

    @abstractmethod
    def validate(self) -> None:
        """
        Verify that the input has all the required frames and that they are valid themselves.
        There is no default logic, implementation is fully deferred to derived classes.
        """

    @abstractmethod
    def load_structure(self) -> None:
        """
        Load the input structure and store as data items, but without data yet.
        """

    @abstractmethod
    def load_data(self, extension: int | str = None) -> Union[cpl.core.ImageList, cpl.core.Image, cpl.core.Table]:
        """
        Load the actual data content and return it.
        """

    def as_dict(self) -> dict[str, Any]:
        return {
            'item': self.Item,
            'required': self.required(),
        }

    @classmethod
    @final
    def extended_description_line(cls, **tags) -> str:
        """
        Produce an extended description line for the man page.

        Tag placeholders the recipe pins statically (passed as `tags`, e.g. a detector
        set by a mixin) are resolved here; the rest remain as `{placeholders}`,
        because only the input data can determine them.
        """
        assert cls.Item is not None, f"{cls.__qualname__} has no item"
        assert cls.Item.name() is not None, f"{cls.Item.__qualname__} has no name"
        assert cls.Item.description() is not None, f"{cls.Item.__qualname__} has no description defined"

        return (f"  {partial_format(cls.Item.name(), **tags):<36}"
                f"[{cls._multiplicity}]{' (optional)' if not cls._required else '           '}"
                f" {partial_format(cls.Item.description(), **tags)}")

    @property
    @abstractmethod
    def contents(self):
        """
        Return the data contents (either the item or items)
        """

    @abstractmethod
    def valid_frames(self) -> cpl.ui.FrameSet:
        """
        Return a FrameSet containing all valid, used frames.
        This is abstract as it differs significantly for Single and Multiple Inputs.
        """

    @abstractmethod
    def used_frames(self) -> cpl.ui.FrameSet:
        """
        Return a FrameSet containing all used frames.
        """

    @abstractmethod
    def use(self) -> None:
        """
        Mark the input as used
        """