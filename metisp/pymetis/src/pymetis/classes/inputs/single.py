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

from typing import Any

import cpl

from cpl.core import Msg

from pymetis.classes.dataitems import DataItem
from pymetis.classes.inputs.base import PipelineInput


class SinglePipelineInput(PipelineInput):
    """
    A pipeline input that expects a single frame to be present.
    """
    _multiplicity: str = '1'

    def __init__(self,
                 frameset: cpl.ui.FrameSet):                       # Any other args

        self.frame: cpl.ui.Frame | None = None
        super().__init__(frameset)

    def load(self, frameset: cpl.ui.FrameSet):
        if len(frameset) > 1:
            Msg.warning(self.__class__.__name__,
                        f"Expected {self._multiplicity} frames, but found {len(frameset)}!")
        else:
            Msg.debug(self.__class__.__name__,
                      f"Found a {self.Item.__qualname__} frame {frameset[0].file}")
            self.frame = frameset[0]


    def validate(self):
        """
        Run all the required instantiation time checks
        """
        Msg.debug(self.__class__.__qualname__,
                  f"Input tag parameters: {self.tag_parameters}")
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
                    f"{self.__class__.__qualname__}: no {self.title()} frame "
                    f"({self.tags().pattern}) found in the frameset.")
            else:
                Msg.debug(self.__class__.__qualname__,
                          f"No {self.title()} frame found, but not required.")
        else:
            Msg.debug(self.__class__.__qualname__,
                      f"Found a {self.title()} frame {frame.file}")

    def as_dict(self) -> dict[str, Any]:
        return super().as_dict() | {
            'frame': str(self.frame),
        }

    def valid_frames(self) -> cpl.ui.FrameSet:
        if self.frame is None:
            # This may happen for non-required inputs
            return cpl.ui.FrameSet()
        else:
            return cpl.ui.FrameSet([self.frame])
