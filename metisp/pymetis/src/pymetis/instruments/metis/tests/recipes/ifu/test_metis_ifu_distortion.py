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

import types

import cpl
import pytest

from pymetis.engine.recipes import Recipe, RecipeImpl
from pymetis.instruments.metis.recipes.ifu.metis_ifu_distortion import (MetisIfuDistortion as Recipe,
                                                      MetisIfuDistortionImpl as Impl)
from tests.classes import BaseRecipeTest, BaseInputSetTest, BaseProductSetTest


recipe_name = r'metis_ifu_distortion'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.sof'


class TestRecipe(BaseRecipeTest):
    Recipe = Recipe


class TestInputSet(BaseInputSetTest):
    Impl = Impl


class TestProductSetDistortionTable(BaseProductSetTest):
    Product = Impl.ProductSet.DistortionTable


class TestProductSetDistortionReduced(BaseProductSetTest):
    Product = Impl.ProductSet.DistortionReduced


class TestEmptyTraceGuard:
    """The recipe must not emit a distortion table that describes no slice at all."""

    #: All `_verify_any_trace_found` touches. Instantiating the real implementation
    #: would need a complete input set with raw frames on disk, which says nothing
    #: about the guard itself.
    stub = types.SimpleNamespace(name=recipe_name)

    @staticmethod
    def output(*counts: int) -> list[dict]:
        return [{'n_traces': count} for count in counts]

    def test_raises_when_no_detector_yields_a_trace(self) -> None:
        with pytest.raises(cpl.core.DataNotFoundError, match="No slice was traced"):
            Impl._verify_any_trace_found(self.stub, self.output(0, 0, 0, 0))

    def test_passes_when_a_single_detector_yields_a_trace(self) -> None:
        Impl._verify_any_trace_found(self.stub, self.output(0, 1, 0, 0))


class TestDegreeOrBest:
    """`trace` accepts a fixed degree or `best`; CPL parameters are singly typed."""

    @pytest.mark.parametrize("value, expected", [("2", 2), (2, 2), (" 3 ", 3), ("0", 0)])
    def test_integers_come_back_as_integers(self, value, expected) -> None:
        assert Impl._degree_or_best(value) == expected

    @pytest.mark.parametrize("value", ["best", " best "])
    def test_best_is_passed_through(self, value) -> None:
        assert Impl._degree_or_best(value) == 'best'

    @pytest.mark.parametrize("value", ["", "2.5", "worst", "best2"])
    def test_anything_else_is_rejected(self, value) -> None:
        with pytest.raises(cpl.core.IllegalInputError, match="integer or 'best'"):
            Impl._degree_or_best(value)
