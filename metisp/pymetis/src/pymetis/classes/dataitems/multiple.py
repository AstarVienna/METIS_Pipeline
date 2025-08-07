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
from cpl.core import Msg
from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem, ImageDataItem, TableDataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class MultipleDataItem(DataItem, abstract=True):
    _frame_type = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 header: cpl.core.PropertyList,
                 **extensions):
        super().__init__(header)

        self.extensions = extensions
        for key, ext in self.extensions.items():
            self.__setattr__(key, ext)

    @classmethod
    def load_from_frame(cls, frame: cpl.ui.Frame):
        Msg.debug(cls.__qualname__, f"Now loading multiplet {frame.file}")
        header = cpl.core.PropertyList.load(frame.file, extension)
        return cls(header, table)

    def save(self,
                   recipe: 'PipelineRecipe',
                   parameters: ParameterList,
                   *,
                   output_file_name: str = None) -> None:

        # TODO: to_cplui is broken in pyesorex 1.0.3, so it is removed; need to put it back.
        parameters = cpl.ui.ParameterList([p for p in parameters])
        # parameters = cpl.ui.ParameterList([Parameter.to_cplui(p) for p in parameters])

        cpl.dfs.save_propertylist(
            recipe.frameset,
            parameters,
            recipe.used_frames,
            recipe.name,
            self.properties,
            PIPELINE,
            self.file_name(output_file_name),
            header=self.header,
        )

        for key, ext in self.extensions.items():
            ext.save(key, cpl.core.PropertyList(), cpl.core.io.EXTEND)


class MultipleImageDataItem(ImageDataItem, MultipleDataItem, abstract=True):
    def __init__(self,
                 primary_header: cpl.core.PropertyList,
                 **extensions: cpl.core.Image):
        super().__init__(primary_header, **extensions)


class MultipleTableDataItem(TableDataItem, MultipleDataItem, abstract=True):
    def __init__(self,
                 primary_header: cpl.core.PropertyList,
                 *extensions: cpl.core.Table):
        super().__init__(primary_header, **extensions)