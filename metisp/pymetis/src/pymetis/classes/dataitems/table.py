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
from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class TableDataItem(DataItem, abstract=True):
    _frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.TABLE

    def __init__(self,
                 header: cpl.core.PropertyList,
                 frame: cpl.ui.Frame):
        super().__init__(header, frame)
        self.table: cpl.core.Table = cpl.core.Table.empty(3) # ToDo Change this

    def save(self,
             recipe: 'PipelineRecipe',
             parameters: ParameterList,
             *,
             output_file_name: str = None) -> None:

        # TODO: to_cplui is broken in pyesorex 1.0.3, so it is removed; need to put it back.
        parameters = cpl.ui.ParameterList([p for p in parameters])
        # parameters = cpl.ui.ParameterList([Parameter.to_cplui(p) for p in parameters])

        assert isinstance(self.table, cpl.core.Table), \
            f"Attribute `{self}.table` is not a table, but {type(self.table)}"

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
