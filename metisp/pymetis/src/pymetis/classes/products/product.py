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

import re

from abc import ABC
from typing import Any, final

import cpl
from cpl.core import Msg

from pymetis.classes.headers.header import Header

PIPELINE = r'METIS'


class PipelineProduct(ABC):
    """
        The abstract base class for a pipeline product:
        one FITS file with associated headers and a frame
    """

    # Global defaults for all Products
    group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.PRODUCT        # ToDo: Is this a sensible default?
    level: cpl.ui.Frame.FrameLevel = None
    frame_type: cpl.ui.Frame.FrameType = None

    # Product metadata.
    # The standard way of defining them is to override the private class attribute;
    # the default @classmethod with the same name (without the underscore) just returns its value.
    # If it depends on other attributes, override the corresponding @classmethod.
    # All methods dealing with these should relate to the class, not instances!
    _tag: str = None
    _oca_keywords: {Header} = None
    _description: str = None

    # Use this regex to verify that the product tag is correct.
    # This base version only verifies it is ALL_CAPS_WITH_UNDERSCORES, feel free to override
    _regex_tag: re.Pattern = re.compile(r"^[A-Z]+[A-Z0-9_]+[A-Z0-9]+$")

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image):
        self.recipe: 'MetisRecipeImpl' = recipe_impl
        self.header: cpl.core.PropertyList = header
        self.image: cpl.core.Image = image
        self.properties = cpl.core.PropertyList()

        self._used_frames: cpl.ui.FrameSet | None = None

        # Raise a NotImplementedError in case a derived class forgot to set a class attribute
        if self.tag is None:
            raise NotImplementedError(f"Products must define 'tag', but {self.__class__.__qualname__} does not")

        if self.group is None:
            raise NotImplementedError(f"Products must define 'group', but {self.__class__.__qualname__} does not")

        if self.level is None:
            raise NotImplementedError(f"Products must define 'level', but {self.__class__.__qualname__} does not")

        if self.frame_type is None:
            raise NotImplementedError(f"Products must define 'frame_type', but {self.__class__.__qualname__} does not")

        if self.category is None:
            raise NotImplementedError(f"Products must define 'category', but {self.__class__.__qualname__} does not")

        self.add_properties()

    def add_properties(self) -> None:
        """
        Hook for adding custom properties.
        Currently only adds ESO PRO CATG to every product,
        but derived classes are more than welcome to add their own stuff.
        Do not forget to call super().add_properties() then.
        """
        self.properties.append(
            cpl.core.Property(
                "ESO PRO CATG",         # Martin suspects this means ESO product category
                cpl.core.Type.STRING,
                self.category,
            )
        )

    def as_frame(self) -> cpl.ui.Frame:
        """ Create a CPL Frame from this Product """
        return cpl.ui.Frame(
            file=self.output_file_name,
            tag=self.tag(),
            group=self.group,
            level=self.level,
            frameType=self.frame_type,
        )

    def as_dict(self) -> dict[str, Any]:
        """ Return a dictionary representation of this Product """
        return {
            'tag': self.tag(),
            'group': self.group,
            'level': self.level,
        }

    @final
    def __str__(self) -> str:
        return f"{self.__class__.__qualname__} ({self.tag()})"

    @final
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
        cpl.dfs.save_image(
            self.recipe.frameset,       # All frames for the recipe
            self.recipe.parameters,     # The list of input parameters
            self.recipe.valid_frames,   # The list of frames actually used FixMe currently not working as intended
            self.image,                 # Image to be saved
            self.recipe.name,           # Name of the recipe
            self.properties,            # Properties to be appended
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )

    @property
    def category(self) -> str:
        """
        Return the category of this product.

        By default, the tag is the same as the category. Feel free to override if needed.
        """
        return self.tag()

    @property
    def output_file_name(self) -> str:
        """
        Form the output file name.
        By default, this should be just the category with ".fits" appended. Feel free to override if needed.
        """
        return f"{self.category}.fits"

    @property
    def used_frames(self) -> cpl.ui.FrameSet:
        """
        Returns
        -------
            cpl.ui.FrameSet:    List of all frames actually used by the product.
        """
        return self._used_frames

    @classmethod
    def tag(cls) -> str:
        return cls._tag

    @classmethod
    def description(cls) -> str:
        return cls._description

    @classmethod
    def description_line(cls) -> str:
        return f"    {cls.tag():<75s}{cls.description() or '<not defined>'}"

