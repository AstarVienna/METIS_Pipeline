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
import subprocess
import pytest

from abc import ABC
from pathlib import Path

import cpl

from pymetis.inputs import PipelineInputSet, MultiplePipelineInput
from pymetis.base.product import PipelineProduct

root = Path(os.path.expandvars("$SOF_DIR"))


class BaseProductTest(ABC):
    product = None

    def test_is_it_even_a_product(self):
        assert issubclass(self.product, PipelineProduct)

    def test_does_it_have_a_group(self):
        assert self.product.group is not None

    def test_does_it_have_a_level(self):
        assert self.product.level is not None

    def test_does_it_have_a_frame_type(self):
        assert self.product.frame_type is not None


class BaseInputSetTest(ABC):
    """
    A set of basic tests common for all InputSets
    """
    impl = None
    count = None

    def test_is_an_inputset(self):
        assert issubclass(self.impl.InputSet, PipelineInputSet)

    def test_is_not_abstract(self):
        assert not inspect.isabstract(self.impl.InputSet)

    def test_can_load_and_verify(self, load_frameset, sof):
        instance = self.impl.InputSet(load_frameset(sof))
        assert instance.validate() is None, f"InputSet {instance} did not validate"

    def test_all_inputs(self, load_frameset, sof):
        # We should really be testing a class here, not an instance
        instance = self.impl.InputSet(load_frameset(sof))
        for inp in instance.inputs:
            assert inp._group is not None
            assert isinstance(inp._title, str)


class RawInputSetTest(BaseInputSetTest):
    def test_is_raw_input_count_correct(self, load_frameset, sof):
        instance = self.impl.InputSet(load_frameset(sof))
        assert len(instance.raw.frameset) == self.count

    def test_inputset_has_raw(self, load_frameset, sof):
        instance = self.impl.InputSet(load_frameset(sof))
        assert isinstance(instance.raw, MultiplePipelineInput)


class BaseRecipeTest(ABC):
    """
    Integration / regression tests for verifying that the recipe can be run
    """
    _recipe = None

    @classmethod
    def _run_pyesorex(cls, name, sof):
        return subprocess.run(['pyesorex', name, root / sof, '--log-level', 'DEBUG'],
                              capture_output=True)

    def test_recipe_can_be_instantiated(self):
        recipe = self._recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_recipe_can_be_run_directly(self, load_frameset, sof):
        instance = self._recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        assert isinstance(instance.run(frameset, {}), cpl.ui.FrameSet)
        # pprint.pprint(instance.implementation.as_dict(), width=200)

    def test_recipe_can_be_run_with_pyesorex(self, name, create_pyesorex):
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe), "Recipe is not a cpl.ui.PyRecipe"
        assert pyesorex.recipe.name == name, f"Recipe name {name} does not match the pyesorex name {pyesorex.recipe.name}"

    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex):
        output = self._run_pyesorex(name, sof)
        assert output.returncode == 0, "Pyesorex exited with non-zero return code"
        assert output.stderr == b"", "Pyesorex exited with non-empty stderr"

    def test_recipe_uses_all_input_frames(self, load_frameset, sof):
        instance = self._recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})
        all_frames = sorted([frame.file for frame in instance.implementation.inputset.frameset])
        used_frames = sorted([frame.file for frame in instance.implementation.inputset.used_frames])
        assert all_frames == used_frames,\
               f"Not all frames were used: {instance.implementation.inputset.used_frames!s}"

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
    @pytest.mark.parametrize("band", ['lm', 'n', 'ifu'])
    def test_recipe_can_be_run_directly(self, load_frameset, band):
        sof = f"{self._recipe._name}.{band}.sof"
        super().test_recipe_can_be_run_directly(load_frameset, sof)

    @pytest.mark.parametrize("band", ['lm', 'n', 'ifu'])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, band, create_pyesorex):
        sof = f"{self._recipe._name}.{band}.sof"
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)


class TargetParamRecipeTest(BaseRecipeTest):
    """
    Tests for recipes whose SOFs also specify target parameters ("SCI" | "STD")
    """
    @pytest.mark.parametrize("target", ['std', 'sci'])
    def test_recipe_can_be_run_directly(self, load_frameset, target):
        sof = f"{self._recipe._name}.{target}.sof"
        super().test_recipe_can_be_run_directly(load_frameset, sof)

    @pytest.mark.parametrize("target", ['std', 'sci'])
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, target, create_pyesorex):
        sof = f"{self._recipe._name}.{target}.sof"
        super().test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex)

