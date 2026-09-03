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
import pprint

from typing import Any

import cpl
from cpl.core import Msg

from pymetis.engine.core.functions.format import partial_format
from pymetis.engine.core.parametrizable import ParametrizableContainer
from pymetis.engine.dataitems.dataitem import DataItem
from pymetis.engine.inputs.input import PipelineInput


class PipelineInputSet(ParametrizableContainer):
    """
    The `PipelineInputSet` class is a utility class for a recipe dealing with the input data.
    It reads and filters the input FrameSet, categorizes the frames by their metadata,
    and finally stores them in its own attributes for further use.
    It also provides verification mechanisms and methods
    for extraction of additional information from the frames.

    Every `RecipeImpl` should have exactly one `InputSet` class
    (possibly but not necessarily shared by multiple recipes).
    Currently, we define them as internal classes of the corresponding `RecipeImpl`,
    but in Python it does not really matter much and does not imply any particular relationship
    between the classes -- it is just a namespacing convention.
    """
    class Meta:
        # The registry root for `specialize` / `promoted` lookups. The items of an
        # InputSet are `PipelineInput` wrappers, so what actually specializes or
        # promotes is each input's `Item` -- a DataItem.
        _T = DataItem

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        # Runs at class creation, so a mis-declared input set fails at import time.
        cls._verify_all_inputs_are_declared()

    def __init__(self, frameset: cpl.ui.FrameSet):
        """
        Filter the input frameset, capture frames that match criteria and assign them
        to the attributes declared on the class (see `list_input_classes`).
        """
        self.inputs: frozenset[PipelineInput] = frozenset() # All inputs for this InputSet.
        self.frameset: cpl.ui.FrameSet = frameset

        # Tag parameter matching this instance of InputSet. Might come from DataItem matches or hard-coded from mixins.
        self.tag_matches: dict[str, str] = {}

        # Now iterate over all declared Inputs, instantiate them and feed them the frameset to filter.
        Msg.debug(self.__class__.__qualname__, "Instantiating inputs")
        for (name, input_class) in self.list_input_classes():
            inp = input_class(frameset)
            setattr(self, name, inp)
            # Add to the set of inputs (for easy iteration over all inputs)
            self.inputs |= {inp}

        for inp in self.inputs:
            Msg.debug(self.__class__.__qualname__,
                      f" - {inp.Item.name()}")

    @classmethod
    def list_input_classes(cls) -> list[tuple[str, type[PipelineInput]]]:
        """
        List the inputs of this input set as (attribute name, input class) pairs.

        Inputs are declared explicitly, as annotated class attributes. The annotation
        names the input class directly: a shared module-level class where the recipe
        needs nothing special, or a nested subclass where it carries configuration::

            class InputSet(PipelineInputSet):
                class RawInput(RawInput):
                    Item = SomeRaw

                raw: RawInput
                master_dark: MasterDarkInput   # the module-level class, as imported

        `__init__` then creates `self.raw` etc. as instances of the annotated class.
        The names are ordinary attributes: IDEs can complete and type them, and a
        grep for `master_dark` finds the declaration. A subclass overrides an input
        simply by re-annotating the attribute with another class; if it defines a
        nested input class, that class must be annotation-bound in the same body
        (`_verify_all_inputs_are_declared` raises if it is not).

        Annotations merge across the MRO; the most derived declaration wins.
        """
        return cls._list_annotated_inputs(cls)

    @staticmethod
    def _list_annotated_inputs(cls: type) -> list[tuple[str, type[PipelineInput]]]:
        """
        The annotation merge behind `list_input_classes`, for an arbitrary class.

        `inspect.get_annotations` with `eval_str` keeps this working when annotations
        arrive as strings (a `from __future__ import annotations` in the declaring
        module, or lazy evaluation on Python >= 3.14): they are evaluated here, with
        the class namespace passed as `locals` so that annotations naming nested
        input classes still resolve. An unresolvable annotation raises NameError
        instead of the input silently vanishing.
        """
        declared: dict[str, type[PipelineInput]] = {}
        for klass in reversed(cls.__mro__):
            annotations = inspect.get_annotations(klass, locals=dict(vars(klass)),
                                                  eval_str=True)
            for name, annotation in annotations.items():
                if isinstance(annotation, type) and issubclass(annotation, PipelineInput):
                    declared[name] = annotation
        return list(declared.items())

    @classmethod
    def _verify_all_inputs_are_declared(cls) -> None:
        """
        Verify that every nested input class is bound to an annotated attribute
        in the class that defines it. Catches a class that defines or overrides
        a nested input class but forgets to (re-)annotate it -- otherwise an
        inherited annotation would silently instantiate something else. Checked
        per defining class, so a subclass may also override an input by
        re-annotation alone, leaving the parent's nested class behind.
        """
        for klass in cls.__mro__:
            members = [
                (name, member) for name, member in vars(klass).items()
                if inspect.isclass(member) and issubclass(member, PipelineInput)
            ]
            if not members:
                continue
            bound = {input_class for _, input_class in cls._list_annotated_inputs(klass)}
            if unbound := [f"{name} ({member.__qualname__})"
                           for name, member in members if member not in bound]:
                raise TypeError(
                    f"{klass.__qualname__}: input class(es) not bound to an annotated "
                    f"attribute: {', '.join(unbound)}. Declare each input explicitly, "
                    f"e.g. `raw: RawInput`, in the body of the class that defines it.")

    @classmethod
    def _bind_input(cls,
                    input_class: type[PipelineInput],
                    item: type[DataItem]) -> type[PipelineInput]:
        """ A subclass of `input_class` carrying `item` as its data item. """
        new_input = type(input_class.__name__, (input_class,), {'Item': item})
        new_input.__qualname__ = f"{cls.__qualname__}.{input_class.__name__}"
        new_input.__module__ = cls.__module__
        return new_input

    @classmethod
    def specialize(cls, **parameters) -> None:
        """
        Specialize this input set statically: resolve every input's `Item` under
        `parameters` and rebind the annotation to an input subclass carrying it.

        Unlike `ParametrizableContainer.specialize`, the rebinding goes through the
        class's own `__annotations__` -- inputs are declared by annotation, and a
        bare class member without one is exactly what
        `_verify_all_inputs_are_declared` rejects.
        """
        Msg.debug(cls.__qualname__,
                  f"Specializing {cls.__qualname__} with {parameters} | {cls.tag_parameters()}")

        rebound = {}
        for attr, input_class in cls.list_input_classes():
            item_class = input_class.Item
            tag = partial_format(item_class._name_template,
                                 **(item_class.tag_parameters() | parameters))
            if tag == item_class._name_template:
                # The parameters resolve nothing in this item's tag: leave the
                # declaration alone rather than binding a pointless clone.
                continue
            item = cls._specialized_item(attr, item_class, **parameters)
            rebound[attr] = cls._bind_input(input_class, item)

        if rebound:
            cls.__annotations__ = inspect.get_annotations(cls) | rebound

    @classmethod
    def promoted(cls, **parameters) -> type['PipelineInputSet']:
        """
        Return a new subclass of this input set with every input's `Item` resolved
        to the concrete registered class matching its fully formatted tag. Mirrors
        `ParametrizableContainer.promoted`, rebinding via annotations; `cls` itself
        is never mutated.

        Note that at run time the inputs already promote themselves per instance,
        from the tags of the loaded frames (see `PipelineInput.__init__`).
        """
        Msg.info(cls.__qualname__,
                 f"Promoting {cls.__qualname__} with {parameters}")

        resolved = {}
        for attr, input_class in cls.list_input_classes():
            item = input_class.Item
            tag = partial_format(item._name_template,
                                 **(item.tag_parameters() | parameters))
            new_item = cls.Meta._T.find(tag)
            if new_item is None:
                raise TypeError(
                    f"Could not promote {input_class.__qualname__}: "
                    f"tag '{tag}' is not registered. "
                    f"Known tags: {cls.Meta._T._registry}")
            resolved[attr] = cls._bind_input(input_class, new_item)

        promoted_cls = type(cls.__name__, (cls,), {'__annotations__': resolved})
        promoted_cls.__qualname__ = cls.__qualname__
        promoted_cls.__module__ = cls.__module__
        return promoted_cls

    @classmethod
    def list_descriptions(cls) -> str:
        return '\n'.join(
            sorted([input_class.extended_description_line() for (name, input_class) in cls.list_input_classes()])
        )

    def validate(self) -> None:
        """
        Validate the inputset:
            - see that all inputs are loaded
                - and that they are themselves valid
                - and that they are processing compatible data (same detector, etc.)
            - parse the tag parameters
                - and assign their values as attributes of the inputset
        """
        Msg.debug(self.__class__.__qualname__,
                  f"Validating the inputset {pprint.pformat(self.inputs)}")

        if len(self.inputs) == 0:
            raise NotImplementedError("PipelineInputSet must define at least one input.")

        try:
            for inp in self.inputs:
                inp.validate()
                Msg.debug(self.__class__.__qualname__, f"Tag parameters for {inp} are {inp.Item.tag_parameters()}")
                self.tag_matches |= inp.Item.tag_parameters()
        except cpl.core.DataNotFoundError as e:
            Msg.error(self.__class__.__qualname__, str(e))


    def print_debug(self, *, offset: int = 0) -> None:
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}--- Detailed class info ---")
        Msg.debug(self.__class__.__qualname__, f"{' ' * offset}{len(self.inputs)} inputs:")

        for inp in self.inputs:
            Msg.debug(self.__class__.__qualname__, f"   {inp.Item.__qualname__:<30s} {inp.contents}")

    def as_dict(self) -> dict[str, Any]:
        """
        Return a dict representation of the input patterns.
        """
        return {
            inp.Item.name(): inp.as_dict()
            for inp in self.inputs
        }

    @property
    def valid_frames(self) -> cpl.ui.FrameSet:
        frameset = cpl.ui.FrameSet()

        for inp in self.inputs:
            frames = inp.valid_frames()
            for frame in frames:
                frameset.append(frame)

        return frameset

    @property
    def used_frames(self) -> cpl.ui.FrameSet:
        """
        Return the frames that actually affect the output anyhow (if a frame is not listed here, the output without
        that frame should be identical

        - [HB]: also includes frames that do not contribute any pixel data,
                for instance, discarded outliers (without them a different frame might be an outlier)
        # FixMe: Currently this only ensures that frames are loaded, not actually used!
        # FixMe: This is not a trivial problem though, maybe it will have to be marked manually everytime.
        """
        return cpl.ui.FrameSet([used for inp in self.inputs for used in inp.used_frames()])
