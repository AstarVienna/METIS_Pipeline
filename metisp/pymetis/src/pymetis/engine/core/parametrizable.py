"""
This file is part of an A* Pipeline.
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
from abc import ABC, ABCMeta
from typing import ClassVar, Self, Optional, final, Any

from cpl.core import Msg

from pymetis.engine.core.format import partial_format


class ParametrizableMeta(ABCMeta):
    """
    Metaclass for the Parametrizable hierarchy.

    Handles:
      - tag parameter merging across the MRO
      - auto-registration of concrete subclasses into their root's _registry
      - skipping abstract classes and partially-specialized templates
    """

    def __new__(mcs, name, bases, namespace, *, abstract=False, **kwargs):
        cls = super().__new__(mcs, name, bases, namespace)  # don't forward kwargs to type
        cls._abstract = abstract

        # Merge _tag_parameters from the MRO, then layer kwargs on top
        merged = {}
        for base in reversed(cls.__mro__):
            params = base.__dict__.get("_tag_parameters")
            if isinstance(params, dict):
                merged.update(params)
        merged.update(kwargs)
        cls._tag_parameters = merged

        if abstract:
            return cls

        # Find the nearest root (a class with its own _registry in __dict__)
        registry = next(
            (base.__dict__["_registry"]
             for base in cls.__mro__
             if "_registry" in base.__dict__),
            None,
        )
        if registry is None:
            return cls  # cls is itself a root, or no root yet

        # Skip if name() can't be computed or still has placeholders
        try:
            tag = cls.name()
        except (AttributeError, AssertionError):
            return cls
        if tag is None or "{" in tag:
            return cls

        if tag in registry:
            Msg.debug(cls.__qualname__, f"{tag} already registered, skipping")
        else:
            registry[tag] = cls
        return cls

    # Class-level helpers — accessed as DataItem.find(...), no @classmethod needed
    def find(cls, key):
        for base in cls.__mro__:
            reg = base.__dict__.get("_registry")
            if reg is not None:
                return reg.get(key)
        return None

    def list_classes(cls):
        return [
            (n, k) for n, k in inspect.getmembers(cls, inspect.isclass)
            if isinstance(k, ParametrizableMeta) and k is not cls
        ]


class Parametrizable(ABC, metaclass=ParametrizableMeta):
    """
    Abstract base class for all types parametrizable by custom tags
    (tiny pieces of data, such as short strings or integers).

    The goal is to be able to define generic interfaces for internal structures and algorithms
    and only override the smallest possible subset of functionality in derived classes.

    The tags can be defined either
    - statically inside the class (e.g. all LM recipes and data items should specify `band = "LM"`
      in the class definition). We call this a 'specialization' of the class.
      Note that partial specialization (defining some of the tags while leaving others unspecified) is possible,
      one might want to define an abstract intermediate class.
    - or obtained from the loaded data in the instances of the class
      (e.g. target can be either "SKY" or "STD" or "SCI", but this is not known until the data are loaded).
      We call this a 'promotion' of the class.

    The end effect should be the same: tag placeholders (`{tag}`) will be replaced by the defined values.
    Only fully specialized classes can be instantiated. This also implies that
    unlike specialization, promotion must be always complete.

    In the context of the pipeline some useful tags may be
        - band: (LM|N|IFU)
        - target: (STD|SCI)
        - detector: (2RG|GEO|IFU)
        - source: (LAMP|TWILIGHT)

    Other parameters might be useful for different pipelines.
    Any strings or types convertible to strings (!s) may be used.
    """
    _tag_parameters: ClassVar[dict[str, Any]] = {}

    @classmethod
    def tag_parameters(cls):
        return cls._tag_parameters


class ParametrizableItem(Parametrizable, abstract=True):
    _name_template: ClassVar[str] = None
    _description_template: ClassVar[Optional[str]] = None

    @classmethod
    def name(cls) -> str:
        assert cls._name_template is not None
        return partial_format(cls._name_template, **cls.tag_parameters())


class ParametrizableContainer(Parametrizable, ABC):
    """
    A ParametrizableContainer is an abstract container class (contains other ParametrizableItem classes)
    and is itself Parametrizable. If specialized / promoted, it propagates the same operation down
    to all its attributes of type "Meta._T".
    """
    class Meta:
        """
        Class for storing class-wide metadata:

        _T: the expected type of the inner items.
        """
        _T: ClassVar[type['ParametrizableItem']] = None

    @classmethod
    def list_descriptions(cls) -> str:
        """ Print formatted descriptions of all available inner items (for man page and such) """
        items = [product_type.extended_description_line(name) for (name, product_type) in cls.list_classes()]
        if len(items) == 0:
            return "--- none ---"
        else:
            return '\n'.join(sorted(items))

    @classmethod
    def specialize(cls, **parameters) -> None:
        """ Specialize this class statically (class-based, from code). """
        Msg.debug(cls.__qualname__,
                  f"Specializing {cls.__qualname__} with {parameters} | {cls.tag_parameters()}")

        for name, item_class in cls.list_classes():
            old_class = item_class
            # Copy the entire type so that we do not mess up the original one
            new_class: cls.Meta._T = type(item_class.__name__, item_class.__bases__, dict(item_class.__dict__))
            new_class.specialize(**(item_class.tag_parameters() | parameters))

            if (klass := cls.Meta._T.find(new_class._name_template)) is None:
                setattr(cls, name, new_class)
                Msg.debug(cls.__qualname__,
                          f"Cannot specialize {old_class.__qualname__} ({old_class.name()}) with {parameters}, "
                          f"had to create a new class {new_class.__qualname__}")
            else:
                setattr(cls, name, klass)
                Msg.debug(cls.__qualname__,
                          f" - {old_class.__qualname__} specialized to {klass.__qualname__} ({klass.name()})")


    @classmethod
    def promote(cls, **parameters) -> None:
        """
        Promote the inner items of this class to appropriate subclasses, as determined from the input data.
        This may be only called after the recipe is initialized.

        May also contain template variables that are not mixed in during class creation.
        For instance, `recipe_{band}_{target}` can specify band=LM, but no target, resulting in partial specialization.
        The target has to be supplied from the actual data then.
        """
        Msg.info(cls.__qualname__,
                 f"Promoting {cls.__qualname__} with {parameters}")

        for name, item in cls.list_classes():
            # Compute the target tag without mutating `item`
            tag = partial_format(item._name_template,
                                 **(item.tag_parameters() | parameters))

            new_class = cls.Meta._T.find(tag)
            if new_class is None:
                raise TypeError(
                    f"Could not promote {item.__qualname__}: "
                    f"tag '{tag}' is not registered. "
                    f"Known tags matching: {[k for k in cls.Meta._T._registry if k.startswith(tag.split('{')[0])]}"
                )
            setattr(cls, name, new_class)

        return

        for name, item in cls.list_classes():
            # Merge the predefined parameters with the new ones
            expanded_parameters = item.tag_parameters() | parameters

            # Try to find a promoted class in the registry
            if (new_class := cls.Meta._T.find(tag := item.specialize(**expanded_parameters))) is None:
                raise TypeError(f"Could not promote class {item}: {tag} is not a registered tag")
            else:
                Msg.debug(cls.__qualname__,
                          f" - {item.__qualname__} ({item.name()}) => {new_class.__qualname__} ({new_class.name()})")

            # Replace the corresponding attribute with the new class
            setattr(cls, name, new_class)


class ParametrizableItem(Parametrizable, ABC):
    """
    Abstract base class for all items parametrizable by tags, such as data items and QC parameters.
    """

    # Name of this item (machine-oriented)
    _name_template: ClassVar[str] = None
    # Description of this item (human-readable)
    _description_template: ClassVar[str] = None

    # Class registry: all derived classes are automatically registered here (unless declared abstract)
    _registry: ClassVar[dict[str, type[Self]]] = {}

    @classmethod
    def specialize(cls, **parameters: str) -> str:
        """
        Specialize the data item's name template with given parameters
        """
        cls._name_template = partial_format(cls._name_template, **parameters)
        return cls._name_template

    @classmethod
    def name(cls) -> str:
        """
        Return the machine-oriented name (tag) of the data item as defined in the DRLD, e.g. "DETLIN_2RG_RAW".
        """
        assert cls._name_template is not None, \
            f"{cls.__qualname__} name template is None"
        return partial_format(cls._name_template, **cls.tag_parameters())

    @classmethod
    def description(cls) -> str:
        """
        Return the human-readable description of the item.
        By default, this just returns the protected internal attribute,
        but can be overridden to build the description from other data, such as band or target.
        """
        assert cls._description_template is not None, \
            f"{cls.__qualname__} description template is None"
        return partial_format(cls._description_template, **cls.tag_parameters())


class KeywordMixin(Parametrizable, ABC):
    """
    Base class for keyword-parametrizable mixins.

    Contains a global registry of such classes, with placeholders as keys
    """

    # Global registry of parametrizable tags in the form {keyword: class},
    # e.g. {'detector': DetectorSpecificMixin, ...}
    # Filled automatically with __init_subclass__.
    _registry: dict[str, type[Self]] = {}

    def __init_subclass__(cls, *, keyword: str = None, **kwargs):
        if keyword is not None:
            cls._registry[keyword] = cls
        super().__init_subclass__(**kwargs)

    @classmethod
    def registry(cls) -> dict[str, type[Self]]:
        """ Class property to access the global registry """
        return cls._registry
