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
import os.path
import pprint
import re
import subprocess
import pytest
from abc import ABC
from pathlib import Path

import cpl

from pymetis.base import MetisRecipeImpl, MetisRecipe
from pymetis.inputs import PipelineInputSet, MultiplePipelineInput, PipelineInput
from pymetis.base.product import PipelineProduct
from pymetis.prefab.rawimage import RawImageProcessor

root = Path(os.path.expandvars("$SOF_DIR"))

bands = ['lm', 'n', 'ifu']
targets = ['std', 'sci']


@pytest.mark.product
class BaseProductTest(ABC):
    _product: type[PipelineProduct] = None

    def test_is_it_even_a_product(self):
        assert issubclass(self._product, PipelineProduct)

    def test_does_it_have_a_group(self):
        assert self._product.group is not None,\
            f"Product group is not defined for {self._product.__qualname__}"

    def test_does_it_have_a_level(self):
        assert self._product.level is not None,\
            f"Product level is not defined for {self._product.__qualname__}"

    def test_does_it_have_a_frame_type(self):
        assert self._product.frame_type is not None,\
            f"Product frame type is not defined for {self._product.__qualname__}"


@pytest.mark.inputset
class BaseInputSetTest(ABC):
    """
    A set of basic tests common for all InputSets
    """
    _impl: MetisRecipeImpl = None

    @pytest.fixture(autouse=True)
    def instance(self, load_frameset, sof):
        return self._impl.InputSet(load_frameset(sof))

    def test_is_an_inputset(self):
        assert issubclass(self._impl.InputSet, PipelineInputSet),\
            f"Class is not an InputSet: {self._impl.InputSet}"

    def test_is_not_abstract(self):
        assert not inspect.isabstract(self._impl.InputSet),\
            f"InputSet is abstract: {self._impl.InputSet}"

    @staticmethod
    def test_has_inputs_and_it_is_a_set(instance):
        assert isinstance(instance.inputs, set),\
            f"Inputs are not a set: {instance.inputs}"

    @staticmethod
    def test_all_inputs_are_registered(instance):
        for name, attr in instance.__dict__.items():
            if isinstance(attr, PipelineInput):
                assert attr in instance.inputs,\
                    f"Input {name} is not registered in inputs"

    @staticmethod
    def test_all_registered_inputs_are_actually_inputs(instance):
        for inp in instance.inputs:
            assert isinstance(inp, PipelineInput),\
                f"Registered input is not an Input: {inp}"

    @staticmethod
    def test_can_load_and_verify(instance):
        assert instance.validate() is None,\
            f"InputSet {instance} did not validate"

    @staticmethod
    def test_all_inputs(instance):
        # We should really be testing a class here, not an instance
        for inp in instance.inputs:
            assert inp._group is not None,\
                f"Input {inp} does not have a group defined"
            assert isinstance(inp._title, str),\
                f"Input {inp} does not have a title defined"

    @staticmethod
    def test_input_has_description(instance):
        for inp in instance.inputs:
            assert inp._description is not None,\
                f"Input {inp} does not have a description defined"

@pytest.mark.inputset
class RawInputSetTest(BaseInputSetTest):
    _impl: RawImageProcessor.InputSet

    @staticmethod
    def test_inputset_has_raw(instance):
        assert isinstance(instance.raw, MultiplePipelineInput)


@pytest.mark.recipe
class BaseRecipeTest(ABC):
    """
    Integration / regression tests for verifying that the recipe can be run
    """
    _recipe: type[MetisRecipe] = None

    @pytest.fixture(autouse=True)
    def frameset(self, load_frameset, sof) -> cpl.ui.FrameSet:
        return cpl.ui.FrameSet(load_frameset(sof))

    @classmethod
    def _run_pyesorex(cls, name, sof) -> subprocess.CompletedProcess:
        return subprocess.run(['pyesorex', name, root / sof, '--log-level', 'DEBUG'],
                              capture_output=True)

    def test_recipe_can_be_instantiated(self) -> None:
        recipe = self._recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe),\
            "Recipe is not a PyRecipe"

    def test_recipe_can_be_run_directly(self, frameset) -> None:
        recipe = self._recipe()
        assert isinstance(recipe.run(frameset, {}), cpl.ui.FrameSet),\
            f"Recipe {recipe} did not return a FrameSet"
        # pprint.pprint(instance.implementation.as_dict(), width=200)

    @pytest.mark.pyesorex
    def test_recipe_can_be_run_with_pyesorex(self, name, create_pyesorex) -> None:
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe),\
            "Recipe is not a cpl.ui.PyRecipe"
        assert pyesorex.recipe.name == name,\
            f"Recipe name {name} does not match the pyesorex name {pyesorex.recipe.name}"

    @pytest.mark.pyesorex
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex) -> None:
        output = self._run_pyesorex(name, sof)
        assert output.returncode == 0,\
            f"Pyesorex exited with non-zero return code {output.returncode}"
        assert output.stderr == b"",\
            "Pyesorex exited with non-empty stderr"

    @pytest.mark.pyesorex
    def test_pyesorex_can_display_manpage(self, name) -> None:
        output = subprocess.run(['pyesorex', '--man-page', name, '--log-level', 'DEBUG'], capture_output=True)
        assert output.returncode == 0, \
            f"`pyesorex --man-page {name}` exited with non-zero return code {output.returncode}: {output.stderr}"

    @pytest.mark.xfail(reason="Future-proofing tests")
    def test_does_author_name_conform_to_standard(self) -> None:
        """Test whether the recipe author's name is in the standard format. TBD what that means."""
        recipe = self._recipe()
        assert re.match(rf"^[\w\- ]+, A\*$", recipe._author), "Author name is not in the standard format"

    def test_recipe_uses_all_input_frames(self, frameset):
        instance = self._recipe()
        instance.run(frameset, {})
        all_frames = sorted([frame.file for frame in instance.implementation.inputset.frameset])
        loaded_frames = sorted([frame.file for frame in instance.implementation.inputset.valid_frames])
        assert loaded_frames == all_frames,\
               f"Not all frames were used: {instance.implementation.inputset.loaded_frames!s} vs {all_frames}"

    def test_all_parameters_have_correct_context(self):
        for param in self._recipe.parameters:
            assert param.context == self._recipe._name,\
                   f"Parameter context of {param.name} differs from recipe name {self._recipe.name}"

    def test_all_parameters_name_starts_with_context(self):
        for param in self._recipe.parameters:
            assert param.name.startswith(self._recipe._name),\
                   f"Parameter name {param.name} does not start with {self._recipe._name}"


class BandParamRecipeTest(BaseRecipeTest):
    """
    Tests for recipes whose SOFs also specify band parameters ("LM" | "N" | "IFU")
    """
    @pytest.mark.parametrize("band", bands)
    def test_recipe_can_be_run_directly(self, load_frameset, band):
        sof = f"{self._recipe._name}.{band}.sof"
        frameset = load_frameset(sof)
        super().test_recipe_can_be_run_directly(frameset)

    @pytest.mark.pyesorex
    @pytest.mark.parametrize("band", bands)
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, band, create_pyesorex):
        sof = f"{self._recipe._name}.{band}.sof"
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)


class TargetParamRecipeTest(BaseRecipeTest):
    """
    Tests for recipes whose SOFs also specify target parameters ("SCI" | "STD")
    """
    @pytest.mark.parametrize("target", targets)
    def test_recipe_can_be_run_directly(self, load_frameset, target):
        sof = f"{self._recipe._name}.{target}.sof"
        frameset = load_frameset(sof)
        super().test_recipe_can_be_run_directly(frameset)

    @pytest.mark.pyesorex
    @pytest.mark.parametrize("target", targets)
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, target, create_pyesorex):
        sof = f"{self._recipe._name}.{target}.sof"
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)
