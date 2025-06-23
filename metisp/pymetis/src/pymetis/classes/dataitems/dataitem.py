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

import inspect
from typing import Any, Optional, Generator, final, Literal

import cpl

from cpl.core import Msg

import pymetis


class DataItem:
    """
    The `DataItem` class encapsulates a single data item: the smallest standalone unit of detector data
    or a product of a recipe.
    """
    _registry: dict[str, type['DataItem']] = {}

    # Printable title of the data item. Not used internally, only for human-oriented output
    _title: str = None                      # No universal title makes sense
    # Actual ID of the data item. Used internally for identification. Should mirror DRLD `name`.
    _name: str = None                       # No universal name makes sense
    # CPL frame group and level
    _frame_group: cpl.ui.Frame.FrameGroup = None  # No sensible default; must be provided explicitly
    _frame_level: cpl.ui.Frame.FrameLevel = None  # No sensible default; must be provided explicitly
    _frame_type: cpl.ui.Frame.FrameType = None
    # Associated detector (maybe this does not make much sense here and should be removed)
    _detector: Optional[str] = None         # Not specific to a detector until determined otherwise
    # Associated band
    _band: Optional[Literal['LM', 'N', 'IFU']] = None

    # Description for man page
    _description: Optional[str] = None      # A verbose string; should correspond to the DRLD description

    _oca_keywords: set[str] = set()

    def __init_subclass__(cls,
                          *,
                          abstract: bool = False,
                          description: str = None,
                          **kwargs):
        """
        Register every subclass of DataItem in a global registry.
        Classes marked as abstract are not registered and should never be instantiated.
        """
        cls.__abstract = abstract
        if not abstract:
            DataItem._registry[cls.name()] = cls

        if description is not None:
            cls._description = description

        super().__init_subclass__(**kwargs)

    def __init__(self):
        if self.__abstract:
            raise TypeError(f"Tried to instantiate an abstract data item {self.__class__.__qualname__}")

    @classmethod
    def title(cls) -> str:
        """
        Return the human-readable title of this data item, e.g. "2RG linearity raw"
        """
        return cls._title

    @classmethod
    def name(cls) -> str:
        """
        Return the machine-oriented name of this data item as defined in the DRLD, e.g. "DETLIN_2RG_RAW".
        By default, it returns `_name`, but may be overridden to build the actual name from other attributes.
        """
        return cls._name

    @classmethod
    def detector(cls) -> Optional[str]:
        """
        Return the detector associated with this data item
        """
        return cls._detector

    @classmethod
    @final
    def frame_group(cls):
        """
        Return the group of this data item. Should not be overridden (at least Martin does not see any reason now).
        """
        return cls._frame_group

    @classmethod
    @final
    def frame_level(cls):
        return cls._frame_level

    @classmethod
    @final
    def frame_type(cls):
        return cls._frame_type

    @classmethod
    def description(cls) -> str:
        """
        Return the description of the data item.
        By default, this just returns the protected internal attribute,
        but can be overridden to build the description from other data, such as band or target.
        """
        return cls._description

    @classmethod
    def oca_keywords(cls):
        """
        Return the OCA keywords of this data item. By default, it's just the value of the protected attribute,
        but feel free to override if necessary.
        """
        return cls._oca_keywords

    @classmethod
    def pro_catg(cls):
        return NotImplemented # ToDo finish

    def __init__(self):
        # Check if it is defined
        if self.title() is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        # Check if frame_group is defined (if not, this gives rise to strange errors deep within CPL
        # that you really do not want to deal with)
        if not self.frame_group:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no defined group!")

        # A list of matched groups from `tags`. Acquisition differs
        # between Single and Multiple, so we just declare it here.
        self.tag_parameters: dict[str, str] = {}

    def as_dict(self) -> dict[str, str]:
        return {
            'title': self.title(),
            'name': self.name(),
            'group': self.frame_group().name,
        }

    def _verify_same_detector_from_header(self) -> None:
        """
        Verification for headers, currently disabled
        """
        detectors = []
        for frame in self.frameset:
            header = cpl.core.PropertyList.load(frame.file, 0)
            try:
                det = header['ESO DPR TECH'].value
                try:
                    detectors.append({
                                         'IMAGE,LM': '2RG',
                                         'IMAGE,N': 'GEO',
                                         'IFU': 'IFU',
                                     }[det])
                except KeyError as e:
                    raise KeyError(f"Invalid detector name! In {frame.file}, ESO DPR TECH is '{det}'") from e
            except KeyError:
                Msg.warning(self.__class__.__qualname__, "No detector (ESO DPR TECH) set!")

        # Check if all the raws have the same detector, if not, we have a problem
        if (detector_count := len(unique := list(set(detectors)))) == 1:
            self._detector = unique[0]
            Msg.debug(self.__class__.__qualname__,
                      f"Detector determined: {self.detector}")
        elif detector_count == 0:
            Msg.warning(self.__class__.__qualname__,
                        "No detectors specified (this is probably fine in skeleton stage)")
        else:
            # raise ValueError(f"Darks from more than one detector found: {set(detectors)}!")
            Msg.warning(self.__class__.__qualname__,
                        f"Frames from more than one detector found: {unique}!")

    @classmethod
    def _pretty_tags(cls) -> str:
        """ Helper method to print `re.Pattern`s in man-page: remove named capture groups' names. """
        return cls.tags().pattern
        # return re.sub(r"\?P<\w+>", "", cls.tags().pattern)

    @classmethod
    def input_for_recipes(cls) -> Generator['PipelineRecipe', None, None]:
        """
        List all PipelineRecipe classes that use this Input.
        Warning: heavy introspection.
        Useful for reconstruction of DRLD input/product cards.
        """
        for (name, klass) in inspect.getmembers(
                pymetis.recipes,
                lambda x: inspect.isclass(x) and x.implementation_class.InputSet is not None
        ):
            for (n, kls) in inspect.getmembers(klass.implementation_class.InputSet, lambda x: inspect.isclass(x)):
                if issubclass(kls, cls):
                    yield klass

    @classmethod
    def product_of_recipes(cls) -> Generator['PipelineRecipe', None, None]:
        """
        List all PipelineRecipe classes that output this as a product.
        Warning: heavy introspection.
        Useful for reconstruction of DRLD input/product cards.
        """
        for (name, klass) in inspect.getmembers(
                pymetis.recipes,
                lambda x: inspect.isclass(x) and x.implementation_class is not None
        ):
            for (n, kls) in inspect.getmembers(klass.implementation_class, lambda x: inspect.isclass(x)):
                if issubclass(kls, cls):
                    yield klass
