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

import pytest
import inspect

from abc import ABC
from typing import Optional

from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, PipelineInput, MultiplePipelineInput


@pytest.mark.inputset
class BaseInputSetTest(ABC):
    """
    A set of basic tests common for all InputSets
    """
    _impl: Optional[type[MetisRecipeImpl]] = None

    @pytest.fixture(autouse=True)
    def instance(self, load_frameset, sof):
        return self._impl.InputSet(load_frameset(sof))

    def test_is_an_inputset(self):
        assert issubclass(self._impl.InputSet, PipelineInputSet), \
            f"Class is not an InputSet: {self._impl.InputSet}"

    def test_is_not_abstract(self):
        assert not inspect.isabstract(self._impl.InputSet), \
            f"InputSet is abstract: {self._impl.InputSet}"

    @staticmethod
    def test_has_inputs_and_it_is_a_set(instance):
        assert isinstance(instance.inputs, set), \
            f"Inputs are not a set: {instance.inputs}"

    @staticmethod
    def test_all_inputs_are_registered(instance):
        for name, attr in instance.__dict__.items():
            if isinstance(attr, PipelineInput):
                assert attr in instance.inputs, \
                    f"Input {name} is not registered in inputs"

    @staticmethod
    def test_all_registered_inputs_are_actually_inputs(instance):
        for inp in instance.inputs:
            assert isinstance(inp, PipelineInput), \
                f"Registered input is not an Input: {inp}"

    @staticmethod
    def test_can_load_and_verify(instance):
        assert instance.validate() is None, \
            f"InputSet {instance} did not validate"

    @staticmethod
    def test_all_inputs(instance):
        # We should really be testing a class here, not an instance
        for inp in instance.inputs:
            assert inp.item() is not None, \
                f"Input {inp} has no data item defined"

            assert isinstance(inp.item()._title, str), \
                f"Data item {inp} does not have a title defined"

            assert inp.item()._description is not None,\
                f"Input {inp} does not have a description defined"


@pytest.mark.inputset
class RawInputSetTest(BaseInputSetTest):
    _impl: type[MetisRecipeImpl]

    @staticmethod
    def test_inputset_has_raw_and_it_is_multiple_input(instance):
        assert isinstance(instance.raw, MultiplePipelineInput)
