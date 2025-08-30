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

import datetime
import inspect
import re
from abc import ABC, abstractmethod
from typing import Optional, Generator, Self, Any, final, Union

import cpl

from cpl.core import Msg, Image as CplImage, Table as CplTable, PropertyList as CplPropertyList
from pyesorex.parameter import Parameter, ParameterList

import pymetis
from pymetis.classes.mixins.base import Parametrizable
from pymetis.utils.format import partial_format

PIPELINE = rf'METIS/1'


class DataItem(Parametrizable, ABC):
    """
    The `DataItem` class encapsulates a single data item:
    the smallest standalone unit of detector data or a product of a recipe.

    Class properties should describe the data item as defined in the DRLD
    (or conversely, we should be at least in theory able to regenerate the DRLD items from code).

    Multiple files with the same tag should correspond to multiple instances of the same DataItem class.
    """
    # Class registry: all derived classes are automatically registered here (unless declared abstract)
    _registry: dict[str, type['DataItem']] = {}

    # Printable title of the data item. Not used internally, only for human-oriented output
    _title_template: str = None                     # No universal title makes sense
    # Actual ID of the data item. Used internally for identification. Should mirror DRLD `name`.
    _name_template: str = None                      # No universal name makes sense
    # Description for man page
    _description_template: Optional[str] = None     # A verbose string; should correspond to the DRLD description

    # CPL frame group and level
    _frame_group: cpl.ui.Frame.FrameGroup = None    # No sensible default; must be provided explicitly
    _frame_level: cpl.ui.Frame.FrameLevel = None    # No sensible default; must be provided explicitly
    _frame_type: cpl.ui.Frame.FrameType = None      # Specialised for image / table / multi-extension data

    _oca_keywords: set[str] = set()

    # HDU schema: a list of types or None
    # By default, only the primary header is present
    _schema: list[Union[None, type[CplImage], type[CplTable]]] = [None]

    # [Hacky] A regex to match the name (mostly to make sure we are not instantiating a partially specialized class)
    __regex_pattern: re.Pattern = re.compile(r"^[A-Z]+[A-Z0-9_]+[A-Z0-9]+$")

    def __init_subclass__(cls,
                          *,
                          abstract: bool = False,
                          **kwargs):
        """
        Register every subclass of DataItem in a global registry.
        Classes marked as abstract are not registered and should never be instantiated.
        # FixMe: Hugo says it might be useful for database views and such. But for now it is so.
        """
        cls.__abstract = abstract

        if cls.__abstract:
            # If the class is not fully specialized, skip it
            Msg.debug(cls.__qualname__,
                      f"Class is abstract, skipping registration")
        else:
            # Otherwise, add it to the global registry
            assert cls.__regex_pattern.match(cls.name()) is not None, \
                (f"Tried to register {cls.__name__} ({cls.name()}) which is not fully specialized "
                 f"(did you mean to set `abstract=True` in the class declaration?)")

            if cls.name().format() in DataItem._registry:
                # If the class is already registered, warn about it and do nothing
                Msg.warning(cls.__qualname__,
                            f"A class with tag {cls.name()} is already registered: {DataItem._registry[cls.name()]}")
            else:
                # Otherwise add it to the registry
                Msg.debug(cls.__qualname__, f"Registered a new class {cls.name()}: {cls}")
                DataItem._registry[cls.name()] = cls

        super().__init_subclass__(**kwargs)

    @classmethod
    @final
    def find(cls, key):
        """
        Try to retrieve the DataItem subclass with tag `key` from the global registry.
        If not found, return None instead (and rely on the caller to raise an exception if this is not desired).
        """
        if key in DataItem._registry:
            return DataItem._registry[key]
        else:
            return None

    @classmethod
    def name_template(cls) -> str:
        return cls._name_template

    @classmethod
    def specialize(cls, **parameters) -> str:
        cls._name_template = partial_format(cls._name_template, **parameters)
        return cls._name_template

    @staticmethod
    def __replace_empty_tags(**parameters):
        """
        Replace all `None` parameters with placeholders.
        Intended for human-readable output in not-fully-specialized recipes, such as man pages.
        For instance, `MASTER_DARK_{detector}_{source}` with parameters `{'source': 'STD', 'detector': None}`
        gets rendered literally as "MASTER_DARK_{detector}_STD".

        ToDo: Change to proper t-strings once Python 3.14 is supported.
        """
        return {key: (f'{{{key}}}' if value is None else value) for key, value in parameters.items()}

    @classmethod
    def title(cls) -> str:
        """
        Return a human-readable title of this data item, e.g. "2RG linearity raw"
        """
        assert cls._title_template is not None, \
            f"{cls.__name__} title template is None"
        return cls._title_template.format(**cls.__replace_empty_tags(**cls.tag_parameters()))

    @classmethod
    def name(cls) -> str:
        """
        Return the machine-oriented name (tag) of the data item as defined in the DRLD, e.g. "DETLIN_2RG_RAW".
        """
        assert cls._name_template is not None, \
            f"{cls.__name__} name template is None"
        return cls._name_template.format(**cls.__replace_empty_tags(**cls.tag_parameters()))

    @classmethod
    def description(cls) -> str:
        """
        Return the description of the data item.
        By default, this just returns the protected internal attribute,
        but can be overridden to build the description from other data, such as band or target.
        """
        assert cls._description_template is not None, \
            f"{cls.__name__} description template is None"
        return cls._description_template.format(**cls.__replace_empty_tags(**cls.tag_parameters()))

    @classmethod
    @final
    def frame_group(cls):
        """
        Return the group of this data item.

        This function should not be overridden.
        """
        return cls._frame_group

    @classmethod
    @final
    def frame_level(cls):
        """
        Return the level of this data item.

        This function should not be overridden.
        """
        return cls._frame_level

    @classmethod
    @final
    def frame_type(cls):
        """
        Return the type of this data item.

        This function should not be overridden.
        """
        return cls._frame_type

    @classmethod
    def oca_keywords(cls):
        """
        Return the OCA keywords of this data item.

        By default, it's just the value of the protected attribute, but feel free to override if necessary.
        """
        return cls._oca_keywords

    @classmethod
    def pro_catg(cls):
        """
        Return the PRO CATG attribute

        Currently the same as _name, and will probably stay like that (and if that is the case it will be removed).
        """
        return cls.name()

    def __init__(self,
                 primary_header: cpl.core.PropertyList = cpl.core.PropertyList(),
                 *hdus):
        if self.__abstract:
            raise TypeError(f"Tried to instantiate an abstract data item {self.__class__.__qualname__}")

        # Check if the title is defined
        if self.title() is None:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no title")

        if self.name() is None:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no name")

        # Check if frame_group is defined (if not, this gives rise to strange errors deep within CPL
        # that you really do not want to deal with)
        if self.frame_group() is None:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no defined group!")

        if self.frame_level() is None:
            raise NotImplementedError(f"DataItem {self.__class__.__qualname__} has no defined level!")

        self._used: bool = False
        self.headers: list[cpl.core.PropertyList] = [primary_header]
        self.hdus = hdus
        # FIXME: temporary to get QC parameters into the product header [OC]

        self.header = primary_header
        if self.header is not None:
            self.properties = self.header
        else:
            self.properties = cpl.core.PropertyList()

        self.add_properties()

        # Instance creation timestamp (should be read-only, used in file name)
        self._created_at: datetime.datetime = datetime.datetime.now()

        Msg.debug(self.__class__.__qualname__,
                  f"Created a data item with a primary header and {len(self.hdus)} HDUs")

    @classmethod
    def load(cls,
             frame: cpl.ui.Frame) -> Self:
        """
        Construct the data item from a frame object.
        """
        klass = cls.find(frame.tag)
        return klass.load_from_frame(frame)

    @classmethod
    def load_from_frame(cls,
                        frame: cpl.ui.Frame) -> Any:
        Msg.debug(cls.__qualname__, f"Now loading {frame.file}")

        header = cpl.core.PropertyList.load(frame.file, 0)
        Msg.debug(cls.__qualname__, f"Schema is {cls._schema}")

        items = []
        for ext, item in enumerate(cls._schema):
            if item is CplImage:
                image = CplImage.load(frame.file, cpl.core.Type.FLOAT, ext)
                items.append(image)
            elif item is CplTable:
                table = CplTable.load(frame.file, ext)
                items.append(table)
            elif item is None:
                header = CplPropertyList.load(frame.file, ext)

        try:
            instance = cls(header, *items)
            return instance
        except TypeError as err:
            Msg.error(cls.__qualname__,
                      f"Cannot instantiate: {err}. Expected {cls._schema}, got {items}")
            raise err

    @property
    def used(self) -> bool:
        """ Return whether this data item is actually used in the product """
        return self._used

    def use(self) -> Self:
        """ Mark this data item as actually used. Return itself to enable method chaining """
        self._used = True
        return self

    def file_name(self, override: Optional[str] = None):
        """
        Get the file name of this data item if used as a product.

        :param: override
        If provided, override the file name. Otherwise, name with formatted timestamp is used.
        """
        # ToDo determine how this should be really formed: timestamp, hash, combination?
        # ToDo Hugo says there is a 56 char limit for file names
        return f"{self.name()}_{self._created_at.strftime("%Y-%m-%dT%H-%M-%S-%f")}.fits" \
            if override is None else override


    def add_properties(self) -> None:
        """
        Hook for adding custom properties.
        Currently only adds ESO PRO CATG to every product,
        but derived classes are more than welcome to add their own stuff.
        Do not forget to call super().add_properties() then.
        """
        self.properties.append(
            cpl.core.Property(
                "ESO PRO CATG",  # Martin suspects this means ESO product category
                cpl.core.Type.STRING,
                self.name(),
            )
        )

    def as_frame(self, filename: Optional[str] = None) -> cpl.ui.Frame:
        """ Create a CPL Frame from this DataItem

        :param: filename
            If not None, override the default file name with this
        """
        assert self.frame_level() is not None, \
            f"Data item {self.__class__.__qualname__} does not define a frame level"

        assert self.frame_type() is not None, \
            f"Data item {self.__class__.__qualname__} does not define a frame type"

        assert self.frame_group() is not None, \
            f"Data item {self.__class__.__qualname__} does not define a frame group"

        return cpl.ui.Frame(
            file=self.file_name(filename),
            tag=self.name(),
            group=self.frame_group(),
            level=self.frame_level(),
            frameType=self.frame_type(),
        )

    @abstractmethod
    def save(self,
             recipe: 'PipelineRecipeImpl',
             parameters: ParameterList,
             *,
             output_file_name: str = None) -> None:
        """ Save the data item to file. Implementation depends on the type of the data. """

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
                      f"Detector determined: {self._detector}")
        elif detector_count == 0:
            Msg.warning(self.__class__.__qualname__,
                        "No detectors specified (this is probably fine in skeleton stage)")
        else:
            # raise ValueError(f"Frames from more than one detector found: {set(detectors)}!")
            Msg.warning(self.__class__.__qualname__,
                        f"Frames from more than one detector found: {unique}!")

    @classmethod
    def input_for_recipes(cls) -> Generator['PipelineRecipe', None, None]:
        """
        List all PipelineRecipe classes that use this Input.
        Warning: heavy introspection.
        Useful for reconstruction of DRLD input/product cards.
        """
        for (name, klass) in inspect.getmembers(
            pymetis.recipes,
            lambda x: inspect.isclass(x) and x.Impl.InputSet is not None):
            for (n, kls) in inspect.getmembers(klass.Impl.InputSet, lambda x: inspect.isclass(x)):
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
                lambda x: inspect.isclass(x) and x.Impl is not None
        ):
            for (n, kls) in inspect.getmembers(klass.Impl, lambda x: inspect.isclass(x)):
                if issubclass(kls, cls):
                    yield klass

    @classmethod
    @final
    def _extended_description_line(cls, name: str = None) -> str:
        """
        Generate a description line for 'pyesorex --man-page'.
        """
        return f"    {cls.name():39s}{cls.description() or '<no description defined>'}"

    def __str__(self):
        return f"{self.name()}"

    def __repr__(self):
        return f"{self.name()}"
