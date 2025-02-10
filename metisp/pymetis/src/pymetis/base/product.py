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

from abc import ABC

import cpl
from cpl.core import Msg

PIPELINE = r"METIS"


class PipelineProduct(ABC):
    """
        The abstract base class for a pipeline product:
        one FITS file with associated headers and a frame
    """

    _tag: str = None
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.PRODUCT        # ToDo: Is this a sensible default?
    _level: cpl.ui.Frame.FrameLevel = None
    _frame_type: cpl.ui.Frame.FrameType = None

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image,
                 **kwargs):
        self.recipe: 'MetisRecipeImpl' = recipe_impl
        self.header: cpl.core.PropertyList = header
        self.image: cpl.core.Image = image
        self.properties = cpl.core.PropertyList()

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

    def add_properties(self):
        """
        Hook for adding properties.
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

    def as_frame(self):
        """ Create a CPL Frame from this Product """
        return cpl.ui.Frame(
            file=self.output_file_name,
            tag=self.tag,
            group=self.group,
            level=self.level,
            frameType=self.frame_type,
        )

    def as_dict(self):
        """ Return a dictionary representation of this Product """
        return {
            'tag': self.tag,
        }

    def __str__(self):
        return f"{self.__class__.__qualname__} ({self.tag})"

    def save(self):
        """ Save this Product to a file """
        Msg.info(self.__class__.__qualname__,
                 f"Saving product file as {self.output_file_name!r}:")
        Msg.info(self.__class__.__qualname__,
                 f"All frames ({len(self.recipe.frameset)}): {sorted([frame.tag for frame in self.recipe.frameset])}")
        Msg.info(self.__class__.__qualname__,
                 f"Used frames ({len(self.recipe.used_frames)}): {sorted([frame.tag for frame in self.recipe.used_frames])}")
        # At least one frame in the recipe frameset must be tagged as RAW!
        # Otherwise, it *will not* save (rite of passage)
        cpl.dfs.save_image(
            self.recipe.frameset,       # All frames for the recipe
            self.recipe.parameters,     # The list of input parameters
            self.recipe.used_frames,    # The list of raw and calibration frames actually used
            self.image,                 # Image to be saved
            self.recipe.name,           # Name of the recipe
            self.properties,            # Properties to be appended
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )

    @property
    def tag(self):
        return self._tag

    @property
    def group(self):
        return self._group

    @property
    def level(self):
        return self._level

    @property
    def frame_type(self):
        return self._frame_type

    @property
    def category(self) -> str:
        """
        Return the category of this product

        By default, the tag is the same as the category. Feel free to override if needed.
        """
        return self.tag

    @property
    def output_file_name(self) -> str:
        """ Form the output file name """
        return f"{self.category}.fits"


class DetectorSpecificProduct(PipelineProduct, ABC):
    detector = None

    def __init__(self,
                 recipe: 'MetisRecipe',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image,
                 *,
                 detector: str = None,
                 **kwargs):

        if detector is not None:
            self.detector = detector

        if self.detector is None:
            raise NotImplementedError(f"Products specific to a detector must define 'detector', but "
                                      f"{self.__class__.__qualname__} does not")

        super().__init__(recipe, header, image, **kwargs)


class TargetSpecificProduct(PipelineProduct, ABC):
    target = None

    def __init__(self,
                 recipe: 'MetisRecipe',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image,
                 *,
                 target: str = None,
                 **kwargs):

        if target is not None:
            self.target = target

        """
            At the moment of instantiation, the `target` attribute must already be set *somehow*. Either
            -   as a class attribute (if it is constant)
            -   from the constructor (if it is determined from the data)
            -   or as a provided property (if it has to be computed dynamically)
        """
        if self.target is None:
            raise NotImplementedError(f"Products specific to a target must define 'target', but "
                                      f"{self.__class__.__qualname__} does not")

        super().__init__(recipe, header, image, **kwargs)


class BandSpecificProduct(PipelineProduct, ABC):
    """
    Product specific to one band. Probably should be merged with all other similar classes.
    """
    band = None

    def __init__(self,
                 recipe: 'MetisRecipe',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image,
                 *,
                 band: str = None,
                 **kwargs):

        if band is not None:
            self.band = band

        """
            At the moment of instantiation, the `band` attribute must already be set *somehow*. Either
            -   as a class attribute (if it is constant)
            -   from the constructor (if it is determined from the data)
            -   or as a provided property (if it has to be computed dynamically)
        """
        if self.band is None:
            raise NotImplementedError(f"Products specific to a target must define 'band', but "
                                      f"{self.__class__.__qualname__} does not")

        super().__init__(recipe, header, image, **kwargs)
