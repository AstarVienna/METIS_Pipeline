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
from typing import Any, Optional, Generator, final, Union

import cpl
from cpl.core import Msg

from pymetis.classes.dataitems.dataitem import DataItem


class PipelineInput:
    """
    This class encapsulates a single logical input to a recipe:
    - either a single file, or a line in the SOF (see SinglePipelineInput)
    - or a set of equivalent files (see MultiplePipelineInput)

    It is a relatively thin wrapper over the inner `Item`.
    """
    Item: type[DataItem] = None
    _title: str = None                      # No universal title makes sense
    _required: bool = True                  # By default, inputs are required to be present
    _detector: Optional[str] = None         # Not specific to a detector until determined otherwise

    _multiplicity: str = None               # Multiplicity of the input, '1' or 'N'

    @staticmethod
    def preprocess_frameset(frameset: cpl.ui.FrameSet) -> dict[str, cpl.ui.FrameSet]:
        """
        Convert a SOF (which is a `list[tuple[filename, tag]]`) to a mapping `tag: list[filename]`
        to make it more convenient for Python.
        """
        result = {}

        for frame in frameset:
            if frame.tag in result:
                result[frame.tag] += [frame]
            else:
                result[frame.tag] = [frame]

        return {
            tag: cpl.ui.FrameSet(frames)
            for tag, frames in result.items()
        }

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
        Actually load the associated frames. Implementation differs between derived classes.
        """
        pass

    @abstractmethod
    def set_cpl_attributes(self):
        """
        Set CPL attributes of loaded frames. ToDO: is this really necessary?
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
        for tag, frames in self.preprocess_frameset(frameset).items():
            cls = DataItem.find(tag)

            if cls is None:
                Msg.warning(self.__class__.__qualname__,
                            f"Found a frame with tag '{tag}', which is not a registered data item. Ignoring.")
                continue
            else:
                if cls == self.Item:
                    Msg.debug(self.__class__.__qualname__,
                              f"Found a fully specialized class {cls.__qualname__} for {tag}, instantiating directly")
                    self.load_frameset(frames)
                elif cls in self.Item.__subclasses__():
                    Msg.debug(self.__class__.__qualname__,
                              f"Found a specialized class {cls.__qualname__} for {tag}, "
                              f"subclassing this {self.Item.__qualname__} and instantiating")
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
        Load the input structure and store as items without data yet.
        """

    @abstractmethod
    def load_data(self, extension: int | str = None) -> Union[cpl.core.ImageList, cpl.core.Image, cpl.core.Table]:
        """
        Load the actual data and return it.
        """

    def print_debug(self, *, offset: int = 0) -> None:
        """
        Print a short description of the tags, optionally with a small offset (N spaces).
        """
        Msg.debug(self.__class__.__qualname__,
                  str(self.Item))

    def as_dict(self) -> dict[str, Any]:
        return {
            'item': self.Item,
            'required': self.required,
        }

    @classmethod
    @final
    def _description_line(cls, name: str = None) -> str:
        """ Produce a description line for man page. """
        return (f"    {cls.Item.name():<60} [{cls._multiplicity}]"
                f"{' (optional)' if not cls._required else '           '} "
                f"{cls.Item.description()}\n{' ' * 84}")

    @classmethod
    @final
    def _extended_description_line(cls, name: str = None) -> str:
        """ Produce ae extended description line for man page. """
        assert cls.Item is not None, f"{cls.__qualname__} has no item"
        assert cls.Item.name() is not None, f"{cls.Item.__qualname__} has no name"
        assert cls.Item.description() is not None, f"{cls.Item.__qualname__} has no description defined"

        return (f"    {cls.Item.name():<24}[{cls._multiplicity}]{' (optional)' if not cls._required else '           '}"
                f" {cls.Item.description()}")
#                f"{f'\n{' ' * 84}'.join([x.__qualname__ for x in set(cls.input_for_recipes())])}")

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