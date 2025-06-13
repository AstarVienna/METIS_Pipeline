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
import re
from abc import abstractmethod, ABC
from typing import Pattern, Any, Optional, Generator, final, Literal

import cpl

from cpl.core import Msg

import pymetis


class DataItem(ABC):
    """
    The `DataItem` class encapsulates a single data item: something that.
    Possible derivatives are an `Input` or a `Product`.

    This class encapsulates a single logical input to a recipe:
    - either a single file, or a line in the SOF (see SinglePipelineInput)
    - or a set of equivalent files (see MultiplePipelineInput)
    """
    # Printable title of the data item. Not used internally, only for human-oriented output
    _title: str = None                      # No universal title makes sense

    # Actual ID of the data item. Used internally for identification.
    _name: str = None                       # No universal name makes sense
    _group: cpl.ui.Frame.FrameGroup = None  # No sensible default; must be provided explicitly
    _detector: Optional[str] = None         # Not specific to a detector until determined otherwise
    _band: Optional[Literal['LM', 'N', 'IFU']] = None
    _description: Optional[str] = None      # Description for man page
    _oca_keywords: set[str] = set()

    _multiplicity: str = '<undefined>'      # Multiplicity of the input, '1' or 'N'

    @classmethod
    def title(cls) -> str:
        """
        Return the human-readable title of this data item, e.g. "2RG linearity raw"
        """
        return cls._title

    @classmethod
    def name(cls) -> str:
        """
        Return the machine-oriented name of this data item as defined in the DRLD, e.g. "DETLIN_2RG_RAW"
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
    def group(cls):
        """
        Return the group of this data item. Should not be overridden (at least Martin does not see any reason now).
        """
        return cls._group

    @classmethod
    def band(cls) -> Optional[Literal['LM', 'N', 'IFU']]:
        """
        Return the spectral band of the data item.
        """
        return cls._band

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

    def __init__(self):
        # Check if it is defined
        if self.title() is None:
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no title")

        # Check if tags are defined...
        if not self.tags():
            raise NotImplementedError(f"Pipeline input {self.__class__.__qualname__} has no defined tag pattern")

        # ...and that they are a re pattern
        if not isinstance(self.tags(), re.Pattern):
            raise TypeError(f"PipelineInput `tags` must be a `re.Pattern`, got '{self.tags()}'")

        # Check is frame_group is defined (if not, this gives rise to strange errors deep within CPL
        # that you really do not want to deal with)
        if not self.group:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no defined group!")

        # A list of matched groups from `tags`. Acquisition differs
        # between Single and Multiple, so we just declare it here.
        self.tag_parameters: dict[str, str] = {}

    def as_dict(self) -> dict[str, Any]:
        return {
            'title': self.title,
            'tags': self.tags(),
            'group': self._group.name,
        }

    @classmethod
    @final
    def _description_line(cls, name: str = None) -> str:
        """ Produce a description line for man page. """
        return (f"    {cls._pretty_tags():<60} [{cls._multiplicity}]"
                f"{' (optional)' if not cls._required else '           '} "
                f"{cls._description}\n{' ' * 84}")

    @classmethod
    @final
    def _extended_description_line(cls, name: str = None) -> str:
        """ Produce ae extended description line for man page. """
        return (f"    {name}\n      {cls._pretty_tags():<60} [{cls._multiplicity}]"
                f"{' (optional)' if not cls._required else '           '} "
                f"{cls._description}\n{' ' * 84}"
                f"{f'\n{' ' * 84}'.join([x.__name__ for x in set(cls.input_for_recipes())])}")

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
                        f"Darks from more than one detector found: {unique}!")

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
