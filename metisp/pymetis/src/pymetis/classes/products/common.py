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

from .product import PIPELINE

from pymetis.classes.products.product import PipelineProduct


class TargetSpecificProduct(PipelineProduct):
    """Products specific to a target. Usually, but not necessarily, (SCI|STD|SKY) and (LAMP|TWILIGHT)"""
    _target: str = None

    @classmethod
    def target(cls) -> str:
        """Return the internal target, or a placeholder for manpage."""
        return cls._target or "{target}"


class DetectorSpecificProduct(PipelineProduct):
    """Products specific to a detector. Usually, but not necessarily, (2RG|GEO|IFU)"""
    _detector: str = None

    @classmethod
    def detector(cls) -> str:
        """Return the internal detector, or a placeholder for manpage."""
        return cls._detector or "{detector}"


class BandSpecificProduct(PipelineProduct):
    """Products specific to a band. Usually, but not necessarily, (LM|N)"""
    _band: str = None

    @classmethod
    def band(cls) -> str:
        """Return the internal band, or a placeholder for manpage."""
        return cls._band or "{band}"
    
class TableProduct(PipelineProduct):
    """Product with table data"""

    def __init__(self,
                    recipe_impl: 'MetisRecipeImpl',
                    header: cpl.core.PropertyList,
                    table: cpl.core.Table):
        super().__init__(recipe_impl, header, image=None)
        self.table: cpl.core.Table = table

    def save(self):
        """ Save this Product to a file """
        Msg.info(self.__class__.__qualname__,
                 f"Saving product file as {self.output_file_name!r}:")
        Msg.info(self.__class__.__qualname__,
                 f"All frames ({len(self.recipe.frameset)}): {sorted([frame.tag for frame in self.recipe.frameset])}")
        Msg.info(self.__class__.__qualname__,
                 f"Loaded frames ({len(self.recipe.valid_frames)}): {sorted([frame.tag for frame in self.recipe.valid_frames])}")
        # Check that the tag matches the generic regex
        assert self._regex_tag.match(self.tag()) is not None, \
            f"Invalid {self.__class__.__qualname__} product tag '{self.tag()}'"
        # At least one frame in the recipe frameset must be tagged as RAW!
        # Otherwise, it *will not* save (rite of passage)
        cpl.dfs.save_table(
            self.recipe.frameset,       # All frames for the recipe
            self.recipe.parameters,     # The list of input parameters
            self.recipe.valid_frames,   # The list of frames actually used FixMe currently not working as intended
            self.table,                 # Table to be saved
            self.recipe.name,           # Name of the recipe
            self.properties,            # Properties to be appended
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )


# FixMe move this to some prefab
class ProductBadpixMapDet(DetectorSpecificProduct):
    group = cpl.ui.Frame.FrameGroup.CALIB  # TBC
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE

    _oca_keywords = {'PRO.CATG'}

    @classmethod
    def description(cls) -> str:
        return rf"Bad pixel map for {cls.detector()}. Also contains detector masks."

    @classmethod
    def tag(cls):
        return rf"BADPIX_MAP_{cls.detector()}"

        # SKEL: copy product keywords from header
    def add_properties(self):
        super().add_properties()
        self.properties.append(self.header)
