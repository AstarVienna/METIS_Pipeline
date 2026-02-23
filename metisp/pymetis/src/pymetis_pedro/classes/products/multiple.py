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

from __future__ import annotations
import cpl


class PipelineMultipleProduct(PipelineProduct):
    """
    PipelineMultiProduct represents products with multi-extensions FITS files.
    The base structure is the PropertyList, with tables or images saved in extensions.
    """
    frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 **extensions):
        super().__init__(recipe_impl, header)

        self.extensions = extensions
        for key, ext in self.extensions.items():
            self.__setattr__(key, ext)

    def save_files(self, parameters: ParameterList) -> None:
        cpl.dfs.save_propertylist(
            self.recipe.frameset,
            parameters,
            self.recipe.used_frames,
            self.recipe.name,
            self.properties,
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )

        for key, ext in self.extensions.items():
            ext.save(self.output_file_name, cpl.core.PropertyList(), cpl.core.io.EXTEND)
