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

from typing import Any, Optional, Self

import cpl

from cpl.core import Msg, Image, ImageList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.inputs.input import PipelineInput


class MultiplePipelineInput(PipelineInput):
    """
    A pipeline input that expects multiple similar frames, such as a raw processor.
    """
    _multiplicity: str = 'N'

    def __init__(self,
                 frameset: cpl.ui.FrameSet):
        self.items: list[DataItem] = []
        self.frameset: Optional[cpl.ui.FrameSet] = cpl.ui.FrameSet()
        super().__init__(frameset)

    def _load_frameset_specific(self, frameset: cpl.ui.FrameSet):
        """
        Load the associated frames.
        A MultiplePipelineInput just assigns all the frames to its attribute.
        """
        self.frameset = frameset
        Msg.debug(self.__class__.__name__,
              f"Found a {self.Item.__qualname__} frameset: {frameset}")


    def load_structure(self) -> None:
        """
        Load the items inside this PipelineInput.

        Returns
        -------
        None
        """
        Msg.info(self.__class__.__name__,
                 f"Loading {self.Item.__qualname__} items")
        if len(self.items) != 0:
            Msg.debug(self.__class__.__qualname__,
                      f"Input already loaded, skipping")
        else:
            self.items = []

            for idx, frame in enumerate(self.frameset):
                Msg.info(self.__class__.__qualname__,
                         f" - loading input frame #{idx}: {frame.file!r}")
                self.items.append(self.Item.load(frame))

            Msg.info(self.__class__.__qualname__,
                     f"Input Items loaded: {self.items}")

            self.use() # FixMe: for now anything that is actually loaded is marked as used

    def load_data(self, extension: int | str = None) -> ImageList:
        """
        Load a list of data items from a FrameSet, all corresponding to the same extension HDU.
        The items should be a homogeneous set of CPL Images.
        # ToDO make sure that there can never be multiple tables, only images

        Parameters
        ----------
        extension:
            The extension of the items to load, can be

            - ``int``, in which case the extension will be fetched by number;
            - ``str``, in which case ``EXTNAME`` will be matched.

        Returns
        -------
        cpl.core.ImageList
            A CPL ``ImageLisŧ`` containing the loaded images.
        """
        self.load_structure()

        Msg.info(self.__class__.__qualname__,
                 f"Loading extension '{extension}' from multiple frames")

        images = [item.load_data(extension) for item in self.items]
        return ImageList(images)

    def set_cpl_attributes(self):
        """
        Set the required CPL attributes from the associated ``DataItem``.
        """
        frameset = cpl.ui.FrameSet()

        for frame in self.frameset:
            frame.group = self.Item.frame_group()
            frame.level = self.Item.frame_level()
            frame.type = self.Item.frame_type()

            Msg.debug(self.__class__.__qualname__,
                      f"Setting CPL attributes: "
                      f"{self.Item.frame_group()} "
                      f"{self.Item.frame_level()} "
                      f"{self.Item.frame_type()}")
            frameset.append(frame)

        self.frameset = frameset

    def validate(self):
        self._verify_frameset_is_not_empty()
        self._verify_same_detector()

    def _verify_frameset_is_not_empty(self) -> None:
        """
        Verification shorthand.

        If a required frameset is not present or empty,
        raise a ``cpl.core.DataNotFoundError`` with the appropriate message.

        Raises
        ------
        cpl.core.DataNotFoundError:
            If the input is required but the frameset is empty
        """
        if (count := len(self.frameset)) == 0:
            if self.required():
                raise cpl.core.DataNotFoundError(
                    f"{self.__class__.__qualname__}: no {self.Item.title()} frames "
                    f"({self.Item.name():s}) found in the frameset."
                )
            else:
                Msg.debug(self.__class__.__qualname__, f"No {self.Item.title()} frames found but not required.")
        else:
            Msg.debug(self.__class__.__qualname__, f"Frameset OK: {count} frame{'s' if count > 1 else ''} found")

    def _verify_same_detector(self) -> None:
        """
        Verify whether all the raw frames originate from the same detector.

        Raises
        ------
        KeyError
            If the found detector name is not a valid detector name.
        ValueError
            If dark frames from more than one detector are found.
        """

    def as_dict(self) -> dict[str, Any]:
        """
        Return a dictionary representation of the input.
        """
        return super().as_dict() | {
            'frame': str(self.frameset),
        }

    @property
    def contents(self):
        return self.frameset

    def use(self) -> Self:
        for item in self.items:
            item.use()
        return self

    def valid_frames(self) -> cpl.ui.FrameSet:
        """
        Return a list of valid frames.

        # FixMe: currently returns everything, but should only return valid frames.
        :return:
            cpl.ui.FrameSet : a list of valid frames
        """
        return self.frameset

    def used_frames(self) -> cpl.ui.FrameSet:
        return cpl.ui.FrameSet([self.frameset[i] for i, item in enumerate(self.items) if item.used])
