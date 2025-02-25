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

from abc import ABC

from pymetis.base import PipelineProduct


@pytest.mark.product
class BaseProductTest(ABC):
    _product: type[PipelineProduct] = None

    @pytest.mark.metadata
    def test_is_it_even_a_product(self):
        assert issubclass(self._product, PipelineProduct)

    @pytest.mark.metadata
    def test_does_it_have_a_group(self):
        assert self._product.group is not None, \
            f"Product group is not defined for {self._product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_have_a_level(self):
        assert self._product.level is not None, \
            f"Product level is not defined for {self._product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_have_a_frame_type(self):
        assert self._product.frame_type is not None, \
            f"Product frame type is not defined for {self._product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_have_a_description(self):
        assert self._product.description is not None, \
            f"No description defined for {self._product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_define_oca_keywords(self):
        assert isinstance(self._product.oca_keywords, set), \
            f"OCA keywords for {self._product.__qualname__} are not a set of strings"
        for kw in self._product.oca_keywords:
            assert isinstance(kw, str), \
                f"OCA keyword '{kw}' is not a string"
