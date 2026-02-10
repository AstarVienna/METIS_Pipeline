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
from typing import ClassVar, Self, Optional, final

from cpl.core import Msg

from pymetis.utils.format import partial_format


class Parametrizable:
    """
    Base class for all types parametrizable by tags, such as
        - band: (LM|N|IFU)
        - target: (STD|SCI)
        - detector: (2RG|GEO|IFU)
        - source: (LAMP|TWILIGHT)

    Other parameters might be useful for different pipelines.

    Currently applies to DataItems, PipelineInputSets and their Mixins.
    """

    _tag_parameters: ClassVar[dict[str, str]] = {}

    @classmethod
    def tag_parameters(cls) -> dict[str, str]:
        """
        Return the tag parameters for this class.
        By default, there are none, but mixins may add their own.
        """
        return cls._tag_parameters

    def __init_subclass__(cls, **kwargs):
        merged = {}
        for base in reversed(cls.__mro__):
            params = base.__dict__.get('_tag_parameters')
            if isinstance(params, dict):
                merged.update(params)

        merged.update(kwargs)

        cls._tag_parameters = merged


class ParametrizableContainer(Parametrizable):
    class Meta:
        _T: ClassVar[type] = None

    @classmethod
    def list_classes(cls):
        """ List all available inner items """
        return inspect.getmembers(cls, lambda x: inspect.isclass(x) and issubclass(x, cls.Meta._T))

    @classmethod
    def list_descriptions(cls) -> str:
        """ Print formatted descriptions of all available inner items (for man page and such) """
        items = [product_type.extended_description_line(name) for (name, product_type) in cls.list_classes()]
        if len(items) == 0:
            return "--- none ---"
        else:
            return '\n'.join(
                sorted(items)
            )

    @classmethod
    def specialize(cls, **parameters) -> None:
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
        Promote the products of this class to appropriate subclasses, as determined from the input data.
        This may be only called after the recipe is initialized.

        May also contain template variables that are not mixed in during class creation.
        For instance, `recipe_{band}_{target}` can specify band=LM, but no target,
        resulting in a partial specialiation. The target has to be supplied from the actual data.)
        """
        Msg.info(cls.__qualname__,
                 f"Promoting {cls.__qualname__} with {parameters}")

        for name, item in cls.list_classes():
            # Try to find a promoted class in the registry
            old_class = item.__qualname__
            old_class_name = item.name()

            if (new_class := cls.Meta._T.find(tag := item.specialize(**(item.tag_parameters() | parameters)))) is None:
                raise TypeError(f"Could not promote class {item}: {tag} is not a registered tag")
            else:
                Msg.debug(cls.__qualname__,
                          f" - {old_class} ({old_class_name}) => "
                          f"{new_class.__qualname__} ({new_class.name()})")

            # Replace the product attribute with the new class
            cls.__class__.__setattr__(cls, name, new_class)


class ParametrizableItem(Parametrizable):
    """
    Abstract base class for all items parametrizable by tags, such as data items and QC parameters.
    """

    _name_template: ClassVar[str] = None
    _description_template: ClassVar[str] = None
    _registry: ClassVar[dict[str, type[Self]]] = {}

    def __init_subclass__(cls,
                          *,
                          abstract: bool = False,
                          **kwargs):
        """
        Register every subclass of ParametrizableItem in a class-global registry, based on its tag parameters.
        """

        super().__init_subclass__(**kwargs)

        cls._abstract = abstract
        if cls.name() in cls._registry:
            # If the class is already registered, warn about it and do nothing.
            Msg.debug(cls.__qualname__,
                      f"A class with tag {cls.name()} is already registered, "
                      f"skipping: {cls._registry[cls.name()].__qualname__}")
        else:
            # Otherwise add the class to the global registry
            Msg.debug(cls.__qualname__,
                      f"Registered a new class {cls.name()}: {cls}")
            cls._registry[cls.name()] = cls

    @staticmethod
    def _replace_empty_tags(**parameters):
        """
        Replace all `None` parameters with placeholders.
        Intended for human-readable output in not-fully-specialized recipes, such as man pages.
        For instance, `MASTER_DARK_{detector}_{source}` with parameters `{'source': 'STD', 'detector': None}`
        gets rendered literally as "MASTER_DARK_{detector}_STD".

        ToDo: Change to proper t-strings once Python 3.14 is supported.
        """
        return {key: (f'{{{key}}}' if value is None else value) for key, value in parameters.items()}

    @classmethod
    @final
    def find(cls, key: str) -> Optional[type[Self]]:
        """
        Try to retrieve the ParametrizableItem subclass with tag ``key`` from the global registry.

        If not found, return ``None`` instead (and leave it to the caller to raise an exception if this is not desired).
        """
        if key in cls._registry:
            return cls._registry[key]
        else:
            return None

    @classmethod
    def specialize(cls, **parameters: str) -> str:
        """
        Specialize the data item's name template with given parameters
        """
        old = cls._name_template
        cls._name_template = partial_format(cls._name_template, **parameters)
        return cls._name_template

    @classmethod
    def name(cls) -> str:
        """
        Return the machine-oriented name (tag) of the data item as defined in the DRLD, e.g. "DETLIN_2RG_RAW".
        """
        assert cls._name_template is not None, \
            f"{cls.__qualname__} name template is None"
        return partial_format(cls._name_template, **cls._replace_empty_tags(**cls.tag_parameters()))

    @classmethod
    def description(cls) -> str:
        """
        Return the description of the item.
        By default, this just returns the protected internal attribute,
        but can be overridden to build the description from other data, such as band or target.
        """
        assert cls._description_template is not None, \
            f"{cls.__name__} description template is None"
        description = partial_format(cls._description_template, **cls._replace_empty_tags(**cls.tag_parameters()))
        return description
