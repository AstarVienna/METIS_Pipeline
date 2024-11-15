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

import cpl

from pymetis.base.input import RecipeInput


class MasterDarkInputMixin(RecipeInput):
    tags_dark: [str] = None

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.master_dark: cpl.core.Image | None = None

        if not self.tags_dark:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_dark`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "master dark"

        if frame.tag in self.tags_dark:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.master_dark, frame, origin=self.__class__.__qualname__, title=title)
            self.master_dark = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.master_dark, "master dark")
        super().verify()
