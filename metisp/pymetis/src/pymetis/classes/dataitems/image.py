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
from typing import Self, Optional

import cpl
from cpl.core import Msg, Image

from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class ImageDataItem(DataItem, abstract=True):
    _frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 header: cpl.core.PropertyList = None,
                 image: cpl.core.Image = None):
        super().__init__(header)
        self.image: cpl.core.Image = image

    @classmethod
    def load_from_frame(cls, frame) -> Self:
        Msg.debug(cls.__qualname__, f"Now loading image {frame.file}")

        header = cpl.core.PropertyList.load(frame.file, 0)
        Msg.debug(cls.__qualname__, f"Schema is {cls._schema}")

        for ext, item in enumerate(cls._schema):
            if item is Image:
                image = cpl.core.Image.load(frame.file, cpl.core.Type.FLOAT, ext)

        instance = cls(header, image)
        return instance

    def save(self,
             recipe: 'PipelineRecipeImpl',
             parameters: ParameterList,
             *,
             output_file_name: str = None) -> None:

        # TODO: to_cplui is broken in pyesorex 1.0.3, so it is removed; need to put it back.
        parameters = cpl.ui.ParameterList([p for p in parameters])
        # parameters = cpl.ui.ParameterList([Parameter.to_cplui(p) for p in parameters])

        assert isinstance(self.image, cpl.core.Image), \
            f"Attribute `{self}.image` is not an image, but {type(self.image).__qualname__}, cannot save"

        Msg.info(self.__class__.__qualname__,
                 f"Saving image {self.file_name(output_file_name)}")
        Msg.debug(self.__class__.__qualname__,
                  f"    used frames {recipe.used_frames}")

        cpl.dfs.save_image(
            recipe.frameset,  # All frames for the recipe
            parameters,
            recipe.used_frames,  # The list of frames actually used  FixMe currently not working as intended
            self.image,  # Image to be saved
            recipe.name,  # Name of the recipe
            self.properties,  # Properties to be appended
            PIPELINE,
            self.file_name(output_file_name),
            header=self.header,
        )

