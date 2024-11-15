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


class GainMapInputMixin(RecipeInput):
    tags_gain_map = [] # ["GAIN_MAP_det"]

    def __init__(self, frameset: cpl.ui.FrameSet, **kwargs):
        self.gain_map: cpl.core.Image | None = None

        if not self.tags_gain_map:
            raise NotImplementedError(f"Inputs with {self.__class__.__qualname__} must define `tags_gain_map`")

        super().__init__(frameset, **kwargs)

    def categorize_frame(self, frame: cpl.ui.Frame) -> None:
        title = "gain map"

        if frame.tag in self.tags_gain_map:
            frame.group = cpl.ui.Frame.FrameGroup.CALIB
            self._override_with_warning(self.gain_map, frame, origin=self.__class__.__qualname__, title=title)
            self.gain_map = frame
        else:
            super().categorize_frame(frame)

    def verify(self) -> None:
        self._verify_frame_present(self.gain_map, "gain map")
        super().verify()


class GainMap2rgInputMixin(GainMapInputMixin):
    """ A gain map for the IFU """
    tags_gain_map = ["GAIN_MAP_2RG"]


class GainMapGeoInputMixin(GainMapInputMixin):
    """ A gain map for the N band """
    tags_gain_map = ["GAIN_MAP_GEO"]