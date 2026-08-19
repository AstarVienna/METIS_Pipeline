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

    def __init__(self, frameset: cpl.ui.FrameSet):
        """
        Filter the input frameset, capture frames that match criteria and assign them
        to the attributes declared on the class (see `list_input_classes`).
        """
        self.inputs: frozenset[PipelineInput] = frozenset() # All inputs for this InputSet.
        self.frameset: cpl.ui.FrameSet = frameset

        # Tag parameter matching this instance of InputSet. Might come from DataItem matches or hard-coded from mixins.
        self.tag_matches: dict[str, str] = {}

        self._verify_all_inputs_are_declared()

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

        Inputs are declared explicitly, as annotated class attributes next to the
        input class they bind::

            class InputSet(PipelineInputSet):
                class RawInput(RawInput):
                    Item = SomeRaw

                raw: RawInput
                master_dark: MasterDarkInput

        `__init__` then creates `self.raw` etc. as instances of the annotated class.
        The names are ordinary attributes: IDEs can complete and type them, and a
        grep for `master_dark` finds the declaration. A subclass that overrides an
        input class must re-annotate the attribute so it binds the override
        (`_verify_all_inputs_are_declared` raises if it does not).

        Annotations merge across the MRO; the most derived declaration wins.
        """
        declared: dict[str, type[PipelineInput]] = {}
        for klass in reversed(cls.__mro__):
            for name, annotation in klass.__dict__.get('__annotations__', {}).items():
                if isinstance(annotation, type) and issubclass(annotation, PipelineInput):
                    declared[name] = annotation
        return list(declared.items())

    @classmethod
    def _verify_all_inputs_are_declared(cls) -> None:
        """
        Verify that every input class attached to this input set is bound to an
        annotated attribute. Catches a subclass that overrides an input class but
        forgets to re-annotate it -- otherwise the parent's version would be
        silently instantiated instead of the override.
        """
        bound = {klass for _, klass in cls.list_input_classes()}
        unbound = [
            f"{name} ({klass.__qualname__})"
            for name, klass in inspect.getmembers(
                cls, lambda x: inspect.isclass(x) and issubclass(x, PipelineInput))
            if klass not in bound
        ]
        if unbound:
            raise TypeError(
                f"{cls.__qualname__}: input class(es) not bound to an annotated attribute: "
                f"{', '.join(unbound)}. Declare each input explicitly, "
                f"e.g. `raw: RawInput`, re-annotating in the class that overrides it.")

    @classmethod
    def list_descriptions(cls) -> str:
        return '\n'.join(
            sorted([product_type.extended_description_line(name) for (name, product_type) in cls.list_input_classes()])
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
