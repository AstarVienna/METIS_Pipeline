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
        assert instance.validate() is None

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
        instance.run(frameset, {})
        #pprint.pprint(instance.implementation.as_dict())

    def test_recipe_can_be_run_with_pyesorex(self, name, create_pyesorex):
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == name

    @staticmethod
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex):
        output = __class__._run_pyesorex(name, sof)
        assert output.returncode == 0
        assert output.stderr == b""

    def test_all_parameters_have_correct_context(self):
        for param in self._recipe.parameters:
            assert param.context == self._recipe._name

    def test_all_parameters_name_starts_with_context(self):
        for param in self._recipe.parameters:
            assert param.name.startswith(self._recipe._name)
