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

from .functions.format import partial_format


class ParametrizableMeta(ABCMeta):
    """
    Metaclass for the Parametrizable hierarchy.

    Handles:
      - tag parameter merging across the MRO
      - auto-registration of concrete subclasses into their root's _registry
      - skipping abstract classes
    """

    def __new__(mcs, name, bases, namespace, *, abstract=False, **kwargs):
        cls = super().__new__(mcs, name, bases, namespace)
        cls._abstract = abstract

        # Merge tag parameters from MRO + class kwargs
        merged = {}
        for base in reversed(cls.__mro__):
            params = base.__dict__.get("_tag_parameters")
            if isinstance(params, dict):
                merged.update(params)
        merged.update(kwargs)
        cls._tag_parameters = merged

        # Resolve template against known parameters
        template = namespace.get("_name_template")
        if template is None:
            template = next(
                (b.__dict__["_name_template"] for b in cls.__mro__[1:]
                 if b.__dict__.get("_name_template") is not None),
                None,
            )
        if template is not None and merged:
            cls._name_template = partial_format(template, **merged)

        if not abstract:
            cls._register()

        return cls

    def __init__(cls, name, bases, namespace, *, abstract=False, **kwargs):
        super().__init__(name, bases, namespace)

    def _register(cls) -> None:
        """Register cls under its current _name_template in the nearest _registry up the MRO."""
        if (key := getattr(cls, "_name_template", None)) is None:
            return
        registry = next(
            (b.__dict__["_registry"] for b in cls.__mro__ if "_registry" in b.__dict__),
            None,
        )
        if registry is None:
            return
        elif key in registry:
            if registry[key] is not cls:
                Msg.warning(cls.__qualname__, f"{key} is already registered to {registry[key].__qualname__}, skipping")
        else:
            registry[key] = cls

    def find(cls, key: str) -> Optional[type]:
        for base in cls.__mro__:
            if (reg := base.__dict__.get("_registry")) is not None:
                return reg.get(key)
        return None

    # This should NOT be a classmethod -- we are in a metaclass!
    def list_classes(cls) -> list[tuple[str, type[Self]]]:
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
    def specialize(cls, **parameters: str) -> str:
        """
        Specialize this class for parameters
        """
        cls._tag_parameters = cls._tag_parameters | parameters
        cls._name_template = partial_format(cls._name_template, **parameters)
        type(cls)._register(cls)  # call the metaclass method
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

            # Re-attempt registration now that the template may be fully resolved
            registry = next(
                (b.__dict__["_registry"] for b in new_class.__mro__ if "_registry" in b.__dict__),
                None,
            )
            if registry is not None and new_class._name_template not in registry:
                registry[new_class._name_template] = new_class

            if (klass := cls.Meta._T.find(new_class._name_template)) is None:
                setattr(cls, name, new_class)
                Msg.debug(cls.__qualname__,
                          f"Cannot specialize {old_class.__qualname__} ({old_class.name()}) with {parameters}, "
                          f"had to create a new class {new_class.__qualname__} ({new_class.name()})")
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
                    f"Known tags matching: {cls.Meta._T._registry}"
                )
            setattr(cls, name, new_class)


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
