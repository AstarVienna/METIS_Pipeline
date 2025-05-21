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
import os
import pprint
import re
import subprocess
import pytest

from abc import ABC
from pathlib import Path

import cpl

from pymetis.classes.recipes import MetisRecipe

root = Path(os.path.expandvars("$SOF_DIR"))

bands = ['lm', 'n', 'ifu']
targets = ['std', 'sci']


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
        assert isinstance(recipe, cpl.ui.PyRecipe), \
            "Recipe is not a PyRecipe"

    def test_recipe_can_be_run_directly(self, frameset) -> None:
        recipe = self._recipe()
        assert isinstance(recipe.run(frameset, {}), cpl.ui.FrameSet), \
            f"Recipe {recipe} did not return a FrameSet"

    @pytest.mark.metadata
    def test_recipe_has_a_valid_as_dict(self, frameset) -> None:
        recipe = self._recipe()
        recipe.run(frameset, {})
        out = pprint.pformat(recipe.implementation.as_dict(), width=200)
        assert isinstance(out, str), \
            f"Recipe {recipe.name} did not return a valid as_dict"

    @pytest.mark.pyesorex
    def test_recipe_can_be_run_with_pyesorex(self, name, create_pyesorex) -> None:
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe), \
            "Recipe is not a cpl.ui.PyRecipe"
        assert pyesorex.recipe.name == name, \
            f"Recipe name {name} does not match the pyesorex name {pyesorex.recipe.name}"

    @pytest.mark.pyesorex
    def test_pyesorex_runs_with_zero_exit_code_and_empty_stderr(self, name, sof, create_pyesorex) -> None:
        output = self._run_pyesorex(name, sof)
        assert output.returncode == 0, \
            f"Pyesorex exited with non-zero return code {output.returncode}"
        assert output.stderr == b"", \
            "Pyesorex exited with non-empty stderr"

    @pytest.mark.pyesorex
    def test_pyesorex_can_display_manpage(self, name) -> None:
        output = subprocess.run(['pyesorex', '--man-page', name, '--log-level', 'DEBUG'], capture_output=True)
        assert output.returncode == 0, \
            f"`pyesorex --man-page {name}` exited with non-zero return code {output.returncode}: {output.stderr}"

    @pytest.mark.metadata
    def test_does_author_name_conform_to_standard(self) -> None:
        """Test whether the recipe author's name is in the standard format. TBD what that means."""
        recipe = self._recipe()
        assert re.match(r"^([\w\- ]+, )?A\*$", recipe._author), \
            "Author name is not in the standard format"

    def test_uses_all_input_frames(self, frameset):
        """Test that the recipe uses all input frames."""
        # FixMe this currently does not actually track usage, just loading
        instance = self._recipe()
        instance.run(frameset, {})
        all_frames = sorted([frame.file for frame in instance.implementation.inputset.frameset])
        loaded_frames = sorted([frame.file for frame in instance.implementation.inputset.valid_frames])
        assert loaded_frames == all_frames, \
            f"Not all frames were used: {instance.implementation.inputset.valid_frames!s} vs {all_frames}"

    @pytest.mark.metadata
    def test_are_matched_keywords_defined(self):
        assert self._recipe._matched_keywords is not None, \
            f"Recipe {self._recipe._name} does not have matched keywords defined"

    @pytest.mark.metadata
    def test_is_algorithm_described(self, create_pyesorex):
        assert self._recipe._algorithm is not None, \
            f"Recipe {self._recipe} does not have an algorithm description"

    @pytest.mark.metadata
    def test_all_parameters_have_correct_context(self):
        for param in self._recipe.parameters:
            assert param.context == self._recipe._name, \
                f"Parameter context of {param.name} differs from recipe name {self._recipe._name}"

    @pytest.mark.metadata
    def test_all_parameters_name_starts_with_context(self):
        for param in self._recipe.parameters:
            assert param.name.startswith(self._recipe._name), \
                f"Parameter name {param.name} does not start with {self._recipe._name}"


class BandParamRecipeTest(BaseRecipeTest):
    """
    Tests for recipes whose SOFs also specify band parameters ("LM" | "N" | "IFU")
    This is just a shorthand to parametrize them.
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
    This is just a shorthand to parametrize them.
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
