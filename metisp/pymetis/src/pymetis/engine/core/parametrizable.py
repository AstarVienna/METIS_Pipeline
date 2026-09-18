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
from typing import ClassVar, Self, Optional, Any

from cpl.core import Msg

from .functions.format import partial_format, placeholders


def _origin(klass: type) -> type:
    """ The hand-written class a specialized clone stands for; a hand-written class itself.
    A clone of a clone (`.specialized(band=...)` then `.specialized(order=...)`) stands for
    the same hand-written class as its parent, so the chain is followed to its end. """
    while (origin := getattr(klass, '_specialized_from', None)) is not None and origin is not klass:
        klass = origin
    return klass


class ParametrizableMeta(ABCMeta):
    """
    Metaclass for the Parametrizable hierarchy.

    Handles:
      - tag parameter merging across the MRO
      - auto-registration of concrete subclasses into their root's _registry
      - skipping abstract classes
    """

    def __new__(mcs, name, bases, namespace, *, abstract=False, register=True, **kwargs):
        """
        `abstract=True` marks a template that must not be instantiated; `register=False`
        keeps a class out of both registries (used for specialized clones whose name
        still has placeholders: they are neither a tag nor a hand-written template).
        """
        cls = super().__new__(mcs, name, bases, namespace)
        cls._abstract = abstract

        template = namespace.get("_name_template")
        if template is None:
            template = next(
                (b.__dict__["_name_template"] for b in cls.__mro__[1:]
                 if b.__dict__.get("_name_template") is not None),
                None,
            )

        # Reject keywords that are neither declared tags (`_valid_tags`, if any are declared)
        # nor placeholders of this very name template: a typo like `bnad='LM'` would
        # otherwise silently create a class that never matches. The second kind are
        # indices such as `LCOEFF{order}`, filled per value rather than from the data.
        if kwargs:
            valid = getattr(cls, "_valid_tags", frozenset())
            if not valid:
                raise TypeError(f"{name}: tag keywords {sorted(kwargs)} given, but no tag axes are declared; "
                                f"the instrument package must set `Parametrizable._valid_tags` "
                                f"(pymetis.instruments.metis.mixins does) before tagged classes are defined.")
            own = placeholders(template)
            if unknown := set(kwargs) - set(valid) - own:
                raise TypeError(f"{name}: unknown tag parameter(s) {sorted(unknown)}, "
                                f"valid tags are {sorted(valid)}"
                                f"{f' and the placeholders of {template!r}' if own else ''}")

        # Merge tag parameters from MRO + class kwargs
        merged = {}
        for base in reversed(cls.__mro__):
            params = base.__dict__.get("_tag_parameters")
            if isinstance(params, dict):
                merged.update(params)
        merged.update(kwargs)
        cls._tag_parameters = merged

        # A mixin may label the value it sets (`_tag_labels = {'target': 'standard star'}`).
        # Labels are kept per tag value on the root, so that a class arriving at the value
        # by specialization or promotion (no mixin in its MRO) reads the same in prose.
        own_labels = namespace.get("_tag_labels")
        if isinstance(own_labels, dict):
            for tag, label in own_labels.items():
                if tag not in merged:
                    raise TypeError(f"{name}: label for tag {tag!r} given, but the class sets no value for it")
                Parametrizable._value_labels.setdefault(tag, {})[merged[tag]] = label

        # Resolve template against known parameters
        if template is not None and merged:
            cls._name_template = partial_format(template, **merged)

        if register:
            cls._register()

        return cls

    def __init__(cls, name, bases, namespace, *, abstract=False, register=True, **kwargs):
        super().__init__(name, bases, namespace)

    def _register(cls) -> None:
        """
        Register cls under its current _name_template with the nearest root up the MRO
        (the class declaring `_registry`).

        A fully resolved name of a concrete class goes to `_registry`, the catalogue of
        tags that data can carry. A name with placeholders left goes to `_templates`
        instead, whether the class is abstract or not: such a class can never own a tag,
        but `ParametrizableItem.specialized` must still find it, so that a hand-written
        partial specialization (e.g. the LM flavour of a `{band}_{target}` item) is used
        in preference to a synthesized clone.

        Hand-written classes own their names exclusively: two hand-written classes resolving
        to the same name is a definition error and raises immediately (this is the tripwire
        for copy-pasted mixin lists). Specialized clones (see `ParametrizableItem
        .specialized`, marked `_specialized_from`) never displace a hand-written owner.
        """
        key = getattr(cls, "_name_template", None)
        if key is None or key == "<unknown>":
            return
        root = next((b for b in cls.__mro__ if "_registry" in b.__dict__), None)
        if root is None:
            return
        if '{' in key:
            registry = root.__dict__["_templates"]
        elif cls._abstract:
            return
        else:
            registry = root.__dict__["_registry"]
        existing = registry.get(key)
        if existing is None or existing is cls:
            registry[key] = cls
            return

        # One name, one class. A clone is not a second class but a stand-in for its template,
        # so a clone may meet the template's other clone or its hand-written leaf, and a leaf
        # may arrive after the template's clone; every other encounter is a collision.
        if hasattr(cls, "_specialized_from"):
            if issubclass(_origin(existing), _origin(cls)):
                return                                  # the owner already stands for this template
        elif hasattr(existing, "_specialized_from") and issubclass(cls, _origin(existing)):
            registry[key] = cls                         # the hand-written leaf displaces the template's clone
            return
        raise TypeError(
            f"Tag collision: '{key}' is claimed by two classes, "
            f"{existing.__module__}.{existing.__qualname__} and {cls.__module__}.{cls.__qualname__}. "
            f"Every tag has exactly one class: if these mean the same thing they must derive from "
            f"the same template, and if they do not, their names must differ."
        )

    def find(cls, key: str) -> Optional[type]:
        """ The concrete class owning the fully resolved tag `key`, if any. """
        return cls._lookup("_registry", key)

    def find_template(cls, key: str) -> Optional[type]:
        """ The hand-written class whose name template (placeholders included) is `key`, if any. """
        return cls._lookup("_templates", key)

    def _lookup(cls, attribute: str, key: str) -> Optional[type]:
        for base in cls.__mro__:
            if (reg := base.__dict__.get(attribute)) is not None:
                if (hit := reg.get(key)) is not None:
                    return hit
        return None

    # This should NOT be a classmethod -- we are in a metaclass!
    def list_classes(cls) -> list[tuple[str, type[Self]]]:
        """
        The parametrizable classes attached to `cls` as public members. Underscored
        attributes are bookkeeping (e.g. `_specialized_from`), never members.
        """
        return [
            (n, k) for n, k in inspect.getmembers(cls, inspect.isclass)
            if isinstance(k, ParametrizableMeta) and k is not cls and not n.startswith('_')
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

    # The set of tag keywords the instrument declares (e.g. {'band', 'detector', ...}).
    # When non-empty, ParametrizableMeta rejects unknown tag keywords at class creation.
    # The instrument package sets this once, before defining its mixins.
    _valid_tags: ClassVar[frozenset[str]] = frozenset()
    _value_labels: ClassVar[dict[str, dict[str, str]]] = {}   # tag -> value -> label, see `tag_labels`

    @classmethod
    def tag_parameters(cls):
        return cls._tag_parameters

    @classmethod
    def tag_labels(cls) -> dict[str, str]:
        """
        The tag parameters as words for prose: a mixin may label its value
        (`TargetStdMixin` labels 'STD' as 'standard star'); values without a label are
        returned as they are. Names and titles use the raw values, descriptions the labels.
        """
        return {key: Parametrizable._value_labels.get(key, {}).get(value, value)
                for key, value in cls._tag_parameters.items()}


class ParametrizableItem(Parametrizable, abstract=True):
    _name_template: ClassVar[str] = None
    _description_template: ClassVar[Optional[str]] = None

    @classmethod
    def specialized(cls, **parameters) -> type[Self]:
        """
        The class this item specializes to under `parameters`.

        Returns `cls` itself when the parameters resolve nothing in its tag, the
        hand-written class owning the resolved tag (or, for a partial resolution, the
        resulting template) where one exists, and otherwise a clone of `cls` with the
        parameters applied. The clone keeps the abstractness of `cls` and, for concrete
        items such as QC parameters, stands in for a leaf class nobody wrote. A concrete
        clone is registered only when its tag is fully resolved (a tag with placeholders
        left can never match anything); an abstract clone is never registered, so
        `promoted()` can tell a missing leaf from a legitimate item. `cls` is never mutated.

        Only the keywords that mean something to `cls` -- declared tags and the
        placeholders of its own name -- are handed to the clone; an index such as `order`
        meant for a sibling item would otherwise be rejected as an unknown tag.
        """
        template = partial_format(cls._name_template, **(cls.tag_parameters() | parameters))
        if template == cls._name_template:
            return cls
        lookup = cls.find_template if '{' in template else cls.find
        if (owner := lookup(template)) is not None:
            # The owner may only be a clone standing for the same hand-written class as `cls`
            # (clones are built from the bases, so two clones of one template are not
            # subclasses of each other) or a hand-written class derived from it (the
            # catalogue leaf); anything else would give the name two meanings.
            if _origin(owner) is _origin(cls) or (not hasattr(owner, '_specialized_from') and issubclass(owner, cls)):
                return owner
            raise TypeError(
                f"{cls.__qualname__} resolves to '{template}', which is owned by the unrelated "
                f"{owner.__module__}.{owner.__qualname__}; every tag has exactly one class.")
        relevant = set(cls._valid_tags) | placeholders(cls._name_template)
        clone = type(cls.__name__, cls.__bases__,
                     dict(cls.__dict__) | {'_specialized_from': cls},
                     abstract=cls._abstract, register='{' not in template,
                     **{key: value for key, value in parameters.items() if key in relevant})
        clone.__qualname__ = cls.__qualname__
        return clone

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
        Tag placeholders are filled with the labels of the tag values where the mixins
        define them (`{target}` reads "standard star", not "STD"), else with the values.
        """
        assert cls._description_template is not None, \
            f"{cls.__qualname__} description template is None"
        return partial_format(cls._description_template, **cls.tag_labels())


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
        items = [product_type.extended_description_line() for (name, product_type) in cls.list_classes()]
        if len(items) == 0:
            return "--- none ---"
        else:
            return '\n'.join(sorted(items))

    @classmethod
    def _derived(cls, namespace: dict[str, Any]) -> type[Self]:
        """ A subclass of this container carrying `namespace`, named like its parent. """
        derived = type(cls.__name__, (cls,), namespace)
        derived.__qualname__ = cls.__qualname__
        derived.__module__ = cls.__module__
        return derived

    @classmethod
    def specialized(cls, **parameters) -> type[Self]:
        """
        A new subclass of this container with every inner item specialized statically
        (class-based, from code), see `ParametrizableItem.specialized`.

        Neither `cls` nor its items are mutated and nothing is registered. Specializing
        an already specialized container starts over from the original, so repeated
        calls cannot stack clones of clones.
        """
        # Only the container's own marker means "I am a specialization"; a hand-written
        # subclass of a specialized container must keep the members it adds.
        origin = cls.__dict__.get('_specialized_from', cls)
        Msg.debug(origin.__qualname__,
                  f"Specializing {origin.__qualname__} with {parameters} | {origin.tag_parameters()}")

        resolved = {name: item.specialized(**parameters) for name, item in origin.list_classes()}
        return origin._derived(resolved | {'_specialized_from': origin})

    @classmethod
    def promoted(cls, **parameters) -> type[Self]:
        """
        Return a new subclass of this container with every inner item resolved to the
        concrete class matching its fully formatted tag, as determined from the input data.

        This deliberately does NOT mutate `cls`: promotion happens per recipe run
        (the tags come from the loaded frames), so the result is assigned to the
        recipe *instance*, never to the shared class. Two runs of the same recipe
        in one process therefore cannot see each other's promoted products.

        The parameters may also contain template variables that are not mixed in during
        class creation. For instance, `recipe_{band}_{target}` can specify band=LM but
        no target, resulting in partial specialization. The target then has to be
        supplied from the actual data.

        A hand-written class owning the resolved tag always wins; where none exists the
        specialized item itself serves, provided it is concrete and no *tag* placeholder
        is left: every keyword in `_valid_tags` must have been determined by now, from the
        class or from the data. Other placeholders are indices (`LCOEFF{order}`,
        `FWHM {nn}`) that the recipe fills per value when it emits the item, and they
        survive promotion. An abstract template with no leaf for the tag is a gap in the
        catalogue and raises.
        """
        Msg.info(cls.__qualname__,
                 f"Promoting {cls.__qualname__} with {parameters}")

        resolved = {}
        for name, item in cls.list_classes():
            candidate = item.specialized(**parameters)
            tag = candidate.name()
            # `specialized` already returns the owner where it resolved something; the lookup
            # covers an item whose name was resolved to begin with. One tag has one class, so
            # the owner can only be the candidate itself or its template's leaf.
            owner = cls.Meta._T.find(tag)
            if owner is not None and not issubclass(_origin(owner), _origin(candidate)):
                raise TypeError(
                    f"Could not promote {item.__qualname__}: the tag '{tag}' is owned by the unrelated "
                    f"{owner.__module__}.{owner.__qualname__}.")
            new_class = owner or candidate
            unresolved = placeholders(tag)
            missing_tags = unresolved & (cls.Meta._T._valid_tags or unresolved)
            if missing_tags or new_class._abstract:
                raise TypeError(
                    f"Could not promote {item.__qualname__} with {parameters}: "
                    f"no concrete class owns the tag '{tag}'"
                    f"{f' (tags {sorted(missing_tags)} remain unresolved)' if missing_tags else ''}."
                )
            resolved[name] = new_class

        return cls._derived(resolved)
