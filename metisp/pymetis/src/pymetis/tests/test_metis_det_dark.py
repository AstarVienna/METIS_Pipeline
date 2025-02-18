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

import cpl

from pymetis.base import MetisRecipe, MetisRecipeImpl, PipelineProduct
from pymetis.recipes.metis_det_dark import (MetisDetDark as Recipe,
                                            MetisDetDarkImpl as Impl)
from generic import BandParamRecipeTest, BaseProductTest, RawInputSetTest

recipe_name = r'metis_det_dark'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.lm.sof'


class TestRecipe(BandParamRecipeTest):
    """ A bunch of simple and stupid test cases... just to see if it does something """
    _recipe: type[MetisRecipe] = Recipe

    def test_fails_with_files_from_multiple_detectors(self, load_frameset):
        with pytest.raises(ValueError):
            instance = self._recipe()
            frameset = cpl.ui.FrameSet(load_frameset("incorrect/metis_det_dark.lm.mixed_raw_detectors.sof"))
            instance.run(frameset, {})

    def test_fails_with_files_from_mismatched_detectors(self, load_frameset):
        with pytest.raises(ValueError):
            instance = self._recipe()
            frameset = cpl.ui.FrameSet(load_frameset("incorrect/metis_det_dark.lm.mismatched_detectors.sof"))
            instance.run(frameset, {})


class TestInputSet(RawInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl


class TestProduct(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductMasterDark
