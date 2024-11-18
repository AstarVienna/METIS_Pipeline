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
import subprocess
from abc import ABC
from pathlib import Path
from pytest import fixture

import cpl

from pymetis.inputs import PipelineInputSet


root = Path(os.path.expandvars("$SOF_DIR"))


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
        assert instance.verify() is None
        assert len(instance.raw.frameset) == self.count


class BaseRecipeTest(ABC):
    _recipe = None

    def test_can_be_created(self):
        recipe = self._recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)

    def test_can_be_run_directly(self, load_frameset, sof):
        instance = self._recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})

    def test_can_be_run_with_pyesorex(self, name, create_pyesorex):
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == name

    @staticmethod
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(name, sof, create_pyesorex):
        output = subprocess.run(['pyesorex', name, root / sof, '--log-level', 'DEBUG'],
                                capture_output=True)
        assert output.returncode == 0
        assert output.stderr == b""
