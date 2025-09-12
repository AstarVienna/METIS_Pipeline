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
from cpl.core import Msg, PropertyList as CplPropertyList

from pyesorex.parameter import ParameterList

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.dataitem import PIPELINE


class ImageDataItem(DataItem, abstract=True):
    _frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 primary_header: cpl.core.PropertyList = None,
                 *hdus: cpl.core.Image):
        super().__init__(primary_header, *hdus)

    def save(self,
             recipe: 'PipelineRecipe',
             parameters: ParameterList,
             *,
             output_file_name: str = None) -> None:

        # TODO: to_cplui is broken in pyesorex 1.0.3, so it is removed; need to put it back.
        parameters = cpl.ui.ParameterList([p for p in parameters])
        # parameters = cpl.ui.ParameterList([Parameter.to_cplui(p) for p in parameters])

        Msg.info(self.__class__.__qualname__,
                 f"Saving image {self.file_name(output_file_name)}")
        Msg.debug(self.__class__.__qualname__,
                  f"Used {len(recipe.used_frames)} frames")
        for frame in recipe.used_frames:
            Msg.debug(self.__class__.__qualname__,
                      f"    {frame}")

        filename = self.file_name(output_file_name)

        assert isinstance(self.header, CplPropertyList), \
            f"{self.header} must be a CplPropertyList, got a {type(self.header)}"

        # Save the header to the primary HDU
        cpl.dfs.save_propertylist(
            recipe.frameset,
            parameters,
            recipe.used_frames,
            recipe.name,
            self.properties,
            PIPELINE,
            filename,
            header=self.header,
        )

        for hdu in self.hdus:
            # Here the signature is (filename, header, mode, dtype=image.dtype)
            hdu.save(filename, self.header, cpl.core.io.EXTEND)