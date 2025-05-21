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

from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.recipes.ifu.metis_ifu_rsrf import (MetisIfuRsrf as Recipe,
                                                MetisIfuRsrfImpl as Impl)
from pymetis.classes.products import PipelineProduct
from pymetis.tests.classes import BaseRecipeTest, BaseProductTest, RawInputSetTest
from pymetis.recipes.ifu.metis_ifu_rsrf import create_ifu_blackbody_image, extract_ifu_1d_spectra

from pytest import approx
import numpy as np
import cpl

recipe_name = r'metis_ifu_rsrf'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return f'{name}.sof'


class TestRecipe(BaseRecipeTest):
    """ A bunch of extremely simple and stupid test cases... just to see if it does something """
    _recipe: type[MetisRecipe] = Recipe


class TestInputSet(RawInputSetTest):
    _impl: type[MetisRecipeImpl] = Impl


class TestProductRsrfBackground(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductRsrfBackground


class TestProductMasterFlatIfu(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductMasterFlatIfu


class TestProductRsrfIfu(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductRsrfIfu


class TestProductBadpixMap(BaseProductTest):
    _product: type[PipelineProduct] = Impl.ProductBadpixMapIfu


class TestBlackBodyImg:
    def test_blackbody_image(self):
        x, y = np.meshgrid(np.arange(512), np.arange(512))

        # wavecal image with spectral dimension along x
        wavecal_data = 3.55 + (x / 512.0 * 0.37)
        wavecal_img = cpl.core.Image(wavecal_data)

        bb_temp = 800   # K
        bb_img = create_ifu_blackbody_image(wavecal_img, bb_temp)

        assert bb_img.get_median() == approx(3.2575, rel=1e-3)
        assert bb_img.get_min() == approx(3.21761, rel=1e-3)
        assert bb_img.get_max() == approx(3.265, rel=1e-3)


class TestExtractTraces:
    def test_extract_traces(self):
        # build a dummy trace list
        trace_list = []
        x_arr = np.arange(512)
        trace = [1.0e-2, 0]
        for i in np.arange(5):
            trace[-1] = (i + 1) * 80.0
            poly_n = len(trace) - 1
            y_arr = \
                [sum([k*x**(poly_n-i) for i, k in enumerate(trace)])
                 for x in x_arr]
            trace_list.append((x_arr, y_arr))

        # create a gradient image
        x, y = np.meshgrid(np.arange(512), np.arange(512))
        data = 30.0 + (x / 512.0 * 100) + (y / 512.0 * 200)
        img = cpl.core.Image(data)

        # extract 1d traces from the image
        traces_1d = extract_ifu_1d_spectra(img, trace_list)

        # something to compare against
        med = [np.median(traces_1d[i]) for i in np.arange(5)]

        assert med == approx([111.73828125, 142.98828125, 174.23828125,
                              205.48828125, 236.73828125])
