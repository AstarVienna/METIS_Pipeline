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

from typing import Optional, Self

import cpl
from cpl.core import Msg, Table
from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class TableDataItem(DataItem, abstract=True):
    _frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.TABLE

    def __init__(self,
                 header: cpl.core.PropertyList,
                 table: cpl.core.Table):
        super().__init__(header)
        self.table: cpl.core.Table = table

    @classmethod
    def load_from_frame(cls, frame: cpl.ui.Frame) -> Self:
        Msg.debug(cls.__qualname__, f"Now loading table {frame.file}")

        header = cpl.core.PropertyList.load(frame.file, 0)

        Msg.debug(cls.__qualname__, f"{cls._schema}")

        for ext, item in enumerate(cls._schema):
            if item is Table:
                table = cpl.core.Table.load(frame.file, ext)

        instance = cls(header, table)
        return instance

    def save(self,
             recipe: 'PipelineRecipe',
             parameters: ParameterList,
             *,
             output_file_name: str = None) -> None:

        # TODO: to_cplui is broken in pyesorex 1.0.3, so it is removed; need to put it back.
        parameters = cpl.ui.ParameterList([p for p in parameters])
        # parameters = cpl.ui.ParameterList([Parameter.to_cplui(p) for p in parameters])

        assert isinstance(self.table, cpl.core.Table), \
            f"Attribute `{self}.table` is not a table, but {type(self.table).__qualname__}, cannot save"

        cpl.dfs.save_table(
            recipe.frameset,  # All frames for the recipe
            parameters,  # The list of input parameters
            recipe.used_frames,  # The list of frames actually used  FixMe currently not working as intended
            self.table,  # Table to be saved
            recipe.name,  # Name of the recipe
            self.properties,  # Properties to be appended
            PIPELINE,
            self.file_name(output_file_name),
            header=self.header,
        )
