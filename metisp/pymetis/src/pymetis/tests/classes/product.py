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
import cpl.ui
import pytest

from abc import ABC

from pymetis.classes.products import (PipelineProduct,
                                      PipelineImageProduct, PipelineTableProduct, PipelineMultipleProduct)


@pytest.mark.product
class BaseProductTest(ABC):
    Product: type[PipelineProduct] = None

    @pytest.mark.metadata
    def test_is_it_even_a_product(self):
        assert issubclass(self.Product, PipelineProduct)

    #@pytest.mark.metadata
    #def test_does_product_group_match(self):
    #    assert self.Product.Item.frame_group() in [cpl.ui.Frame.FrameGroup.PRODUCT, cpl.ui.Frame.FrameGroup.CALIB], \
    #        f"Product group is not PRODUCT or CALIB for {self.Product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_have_a_level(self):
        assert self.Product.Item.frame_level() is not None, \
            f"Product level is not defined for {self.Product.__qualname__}"

    @pytest.mark.metadata
    def test_does_it_have_a_frame_type(self):
        assert self.Product.Item.frame_type() is not None, \
            f"Product frame type is not defined for {self.Product.__qualname__}"


class ImageProductTest(BaseProductTest):
    def test_does_product_type_match(self):
        assert issubclass(self.Product, PipelineImageProduct)
        assert self.Product.frame_type == cpl.ui.Frame.FrameType.IMAGE


class TableProductTest(BaseProductTest):
    def test_does_product_type_match(self):
        assert issubclass(self.Product, PipelineTableProduct)
        assert self.Product.frame_type == cpl.ui.Frame.FrameType.TABLE


class MultipleProductTest(BaseProductTest):
    def test_does_product_type_match(self):
        assert issubclass(self.Product, PipelineMultipleProduct)
        assert self.Product.frame_type == cpl.ui.Frame.FrameType.IMAGE
