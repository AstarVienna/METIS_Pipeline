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
        # All inputs of this InputSet, in declaration order. The order matters: it is the
        # order of `used_frames`, hence of the PRO REC RAW/CAL cards in the product header.
        self.inputs: tuple[PipelineInput, ...] = ()
        self.frameset: cpl.ui.FrameSet = frameset

        # Tag parameter matching this instance of InputSet. Might come from DataItem matches or hard-coded from mixins.
        self.tag_matches: dict[str, str] = {}

        # Now iterate over all declared Inputs, instantiate them and feed them the frameset to filter.
        # A frame that a more specific sibling input claims (IFU_SKY_RAW next to IFU_{target}_RAW)
        # belongs to that sibling: the most specific input wins.
        Msg.debug(self.__class__.__qualname__, "Instantiating inputs")
        input_classes = self.list_input_classes()
        for (name, input_class) in input_classes:
            claimed = frozenset(other.Item for _, other in input_classes
                                if other.Item is not input_class.Item and issubclass(other.Item, input_class.Item))
            inp = input_class(frameset, claimed=claimed)
            setattr(self, name, inp)
            self.inputs += (inp,)

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
    def specialized(cls, **parameters) -> type['PipelineInputSet']:
        """
        A new subclass of this input set with every input's `Item` specialized
        statically under `parameters` (see `ParametrizableItem.specialized`), or
        `cls` itself when the parameters resolve nothing.

        Unlike `ParametrizableContainer.specialized`, the rebinding goes through the
        subclass's `__annotations__` -- inputs are declared by annotation, and a bare
        class member without one is exactly what `_verify_all_inputs_are_declared`
        rejects. Neither `cls` nor its inputs are mutated.
        """
        origin = cls.__dict__.get('_specialized_from', cls)
        Msg.debug(origin.__qualname__,
                  f"Specializing {origin.__qualname__} with {parameters} | {origin.tag_parameters()}")

        rebound = {}
        for attr, input_class in origin.list_input_classes():
            item = input_class.Item.specialized(**parameters)
            if hasattr(item, '_specialized_from'):
                # A clone is a sibling of the hand-written leaves, so no frame's class could
                # ever be a subclass of it: the input would silently match nothing. This
                # happens when the leaf's module is not imported yet; fail at import instead.
                raise TypeError(
                    f"{origin.__qualname__}.{attr}: no hand-written class for {item.name()!r} "
                    f"(specializing {input_class.Item.__qualname__} with {parameters}); "
                    f"import the module defining it before the recipe, or add the class.")
            if item is not input_class.Item:
                rebound[attr] = origin._bind_input(input_class, item)

        if not rebound:
            return origin
        return origin._derived({'__annotations__': rebound, '_specialized_from': origin})

    @classmethod
    def promoted(cls, **parameters) -> type['PipelineInputSet']:
        """
        Return a new subclass of this input set with every input's `Item` resolved
        to the concrete class matching its fully formatted tag. Mirrors
        `ParametrizableContainer.promoted`, rebinding via annotations; `cls` itself
        is never mutated.

        Note that at run time the inputs already promote themselves per instance,
        from the tags of the loaded frames (see `PipelineInput.__init__`).
        """
        Msg.info(cls.__qualname__,
                 f"Promoting {cls.__qualname__} with {parameters}")

        resolved = {}
        for attr, input_class in cls.list_input_classes():
            candidate = input_class.Item.specialized(**parameters)
            tag = candidate.name()
            new_item = cls.Meta._T.find(tag) or candidate
            if '{' in tag or new_item._abstract:
                raise TypeError(
                    f"Could not promote {input_class.__qualname__} with {parameters}: "
                    f"no concrete data item owns the tag '{tag}'.")
            resolved[attr] = cls._bind_input(input_class, new_item)

        return cls._derived({'__annotations__': resolved})

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

        # Declaration order, so that the report is stable; every input is checked, so that
        # the report names everything that is missing rather than the first thing found.
        missing = []
        sources: dict[str, tuple[str, str]] = {}      # tag keyword -> (value, name of the input that set it)
        conflicts = []
        pinned: dict[str, str] = {}                   # tags fixed by an input's declaration, not by its frames
        for name, _ in self.list_input_classes():
            inp = getattr(self, name)
            try:
                inp.validate()
            except cpl.core.DataNotFoundError as e:
                Msg.error(self.__class__.__qualname__, str(e))
                missing.append(str(e))
                continue
            Msg.debug(self.__class__.__qualname__, f"Tag parameters for {inp} are {inp.Item.tag_parameters()}")
            # Only the tags the declaration left open come from the data. An input declared
            # with a leaf (`Item = IfuSkyRaw`, target pinned to SKY by the class) says what it
            # always is: it does not decide the run's tags where the frames do, and it cannot
            # disagree with them -- but where nothing else determines a tag, it fills it in.
            declared = type(inp).Item.tag_parameters()
            from_data = {key: value for key, value in inp.Item.tag_parameters().items() if key not in declared}
            pinned |= declared
            # The frames of one recipe run must agree on every data tag: a GEO gain map next
            # to 2RG darks is a mis-assembled set of frames, not a choice to be made for the user.
            for key, value in from_data.items():
                if key in sources and sources[key][0] != value:
                    conflicts.append(f"{key}: {sources[key][1]} has {sources[key][0]!r}, {name} has {value!r}")
                else:
                    sources.setdefault(key, (value, name))
            self.tag_matches |= from_data

        for key, value in pinned.items():
            self.tag_matches.setdefault(key, value)

        if missing:
            raise cpl.core.DataNotFoundError(
                f"{self.__class__.__qualname__}: {len(missing)} required input(s) not satisfied by the set of frames:\n  "
                + "\n  ".join(missing))
        if conflicts:
            raise cpl.core.IllegalInputError(
                f"{self.__class__.__qualname__}: the frames disagree on the data tags:\n  " + "\n  ".join(conflicts))


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

        The order is significant: CPL DFS takes the standard primary keywords (MJD-OBS,
        DATE-OBS, OBJECT, ...) and the PRO REC1 RAW1 provenance of a product from the first
        RAW frame in this list. The RAW-role inputs therefore come first, in declaration
        order, so that a recipe with two of them (the lamp frames and the WCU OFF frames)
        inherits from the one it declared first, not from whichever the SOF listed first.
        """
        return cpl.ui.FrameSet([used for inp in self.inputs_by_role() for used in inp.used_frames()])

    def inputs_by_role(self) -> tuple[PipelineInput, ...]:
        """ The inputs with the RAW role first, each group in declaration order. """
        return tuple(sorted(self.inputs, key=lambda inp: inp._group != cpl.ui.Frame.FrameGroup.RAW))

    @property
    def primary_frame(self) -> cpl.ui.Frame | None:
        """
        The first frame of the first input declared in the RAW role, or None if the recipe
        has no such input. Passed as `inherit` to CPL DFS, which uses it for the
        HIERARCH ESO keywords of the product header (the standard keywords follow the
        order of `used_frames`, see there).
        """
        for inp in self.inputs_by_role():
            if inp._group != cpl.ui.Frame.FrameGroup.RAW:
                return None
            frames = getattr(inp, 'frameset', None)
            if frames is None:
                frames = [inp.frame] if getattr(inp, 'frame', None) is not None else []
            for frame in frames:
                return frame
        return None
