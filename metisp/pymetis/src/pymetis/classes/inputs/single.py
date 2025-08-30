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

from typing import Any, Union, Optional

import cpl

from cpl.core import Msg

from pymetis.classes.dataitems import DataItem
from pymetis.classes.inputs.input import PipelineInput


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present.
    """
    _multiplicity: str = '1'

    def __init__(self,
                 frameset: cpl.ui.FrameSet):                       # Any other args
        self.item: Optional[DataItem] = None
        self.frame: Optional[cpl.ui.Frame] = None
        super().__init__(frameset)

    def _load_frameset_specific(self, frameset: cpl.ui.FrameSet):
        """
        Load the associated frames.
        A SinglePipelineInput verifies there is exactly one matched frame.
        """
        Msg.debug(self.__class__.__qualname__, f"Loading {frameset}")
        if len(frameset) > 1:
            Msg.warning(self.__class__.__name__,
                        f"Expected a single frame, but found {len(frameset)} of them!")
        else:
            Msg.debug(self.__class__.__name__,
                      f"Found a {self.Item.__qualname__} frame {frameset[0].file}")
            self.frame = frameset[0]

    def load_data(self) -> DataItem:
        Msg.info(self.__class__.__qualname__,
                 f"Loading input frame {self.frame.file!r}")
        self.item = self.Item.load(self.frame)

        Msg.info(self.__class__.__qualname__,
                 f"Item is now {self.item}")

        self.use() # FixMe: for now anything that is actually loaded is marked as used (proof-of-concept)
        return self.item

    def set_cpl_attributes(self):
        self.frame.group = self.Item.frame_group()
        self.frame.level = self.Item.frame_level()
        self.frame.type = self.Item.frame_type()
        Msg.debug(self.__class__.__qualname__,
                  f"Set CPL attributes: {self.Item.frame_group()} {self.Item.frame_level()} {self.Item.frame_type()}")

    def validate(self):
        """
        Run all the required instantiation time checks.
        Currently, we only check whether there is exactly one frame (if required) or at most one (if not required).
        """
        self._verify_frame_present(self.frame)

    def _verify_frame_present(self,
                              frame: cpl.ui.Frame) -> None:
        """
        Verification shorthand: if a required frame is not present, i.e. `None`,
        raise a `cpl.core.DataNotFoundError` with the appropriate message.
        If it is not required, emit a warning but continue.
        """
        if frame is None:
            if self.required():
                raise cpl.core.DataNotFoundError(
                    f"{self.__class__.__qualname__}: no {self.Item.title()} frame "
                    f"({self.Item.name()}) found in the frameset.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"No {self.Item.title()} frame found, but not required.")
        else:
            Msg.debug(self.__class__.__qualname__,
                      f"Found a {self.Item.title()} frame {frame.file}")

    def as_dict(self) -> dict[str, Any]:
        return super().as_dict() | {
            'frame': str(self.frame),
        }

    @property
    def contents(self):
        return self.item

    def use(self) -> None:
        self.item.use()

    def valid_frames(self) -> cpl.ui.FrameSet:
        if self.frame is None:
            # This may happen for non-required inputs
            return cpl.ui.FrameSet()
        else:
            return cpl.ui.FrameSet([self.frame])

    def used_frames(self) -> cpl.ui.FrameSet:
        if self.frame is None or self.item is None:
            return cpl.ui.FrameSet()
        else:
            if self.item.used:
                return cpl.ui.FrameSet([self.frame])
            else:
                return cpl.ui.FrameSet()