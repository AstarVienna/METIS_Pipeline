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

import numpy as np
import pytest

import cpl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      PropertyList as CplPropertyList)

from pymetis.engine.core.classes.image import EnhancedImage
from pymetis.engine.dataitems import Hdu


PREFIX = 'DET1'


def make_image(rows: int = 4, cols: int = 4, value: float = 1.0) -> CplImage:
    """A float `Image` of the given shape filled with `value`."""
    return CplImage(data=np.full((rows, cols), value, dtype=np.float64))


def build_dq_mask(rows: int = 4, cols: int = 4) -> CplImage:
    """An integer `Image` (a bad-pixel mask) with a couple of flagged pixels."""
    mask = np.zeros((rows, cols), dtype=np.int32)
    mask[0, 0] = 1
    mask[1, 2] = 1
    return CplImage(mask, dtype=cpl.core.Type.INT)


def make_imagelist(planes: int = 3, rows: int = 4, cols: int = 4) -> CplImageList:
    """An `ImageList` of `planes` equally-sized float images."""
    return CplImageList([make_image(rows, cols, float(i)) for i in range(planes)])


# ---------- construction ----------


class TestEnhancedImageConstruction:
    def test_all_layers_wrapped_as_named_hdus(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)

        assert isinstance(ei.image, Hdu)
        assert isinstance(ei.error, Hdu)
        assert isinstance(ei.dq, Hdu)

        assert ei.image.name == f'{PREFIX}.SCI'
        assert ei.error.name == f'{PREFIX}.ERR'
        assert ei.dq.name == f'{PREFIX}.DQ'

    def test_extname_written_into_headers(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)

        assert ei.image.header['EXTNAME'].value == f'{PREFIX}.SCI'
        assert ei.error.header['EXTNAME'].value == f'{PREFIX}.ERR'
        assert ei.dq.header['EXTNAME'].value == f'{PREFIX}.DQ'

    def test_imagelist_layers_wrapped_as_named_hdus(self):
        """Layers may be ImageLists (e.g. a stack of coefficient planes); they
        must be wrapped as named HDUs while preserving the ImageList data."""
        ei = EnhancedImage(make_imagelist(3), make_imagelist(3), prefix=PREFIX)

        assert isinstance(ei.image, Hdu)
        assert isinstance(ei.error, Hdu)
        assert ei.image.name == f'{PREFIX}.SCI'
        assert ei.error.name == f'{PREFIX}.ERR'

        assert isinstance(ei.image.data, CplImageList)
        assert isinstance(ei.error.data, CplImageList)
        assert len(ei.image.data) == 3

    def test_optional_layers_default_to_none(self):
        ei = EnhancedImage(make_image(), prefix=PREFIX)

        assert isinstance(ei.image, Hdu)
        assert ei.error is None
        assert ei.dq is None

    def test_missing_headers_default_to_empty(self):
        """
        Headers are optional; omitting them must not crash (`Hdu` unconditionally
        manipulates its header). Exercises every layer, including `dq`.
        """
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)

        assert isinstance(ei.image.header, CplPropertyList)
        assert isinstance(ei.error.header, CplPropertyList)
        assert isinstance(ei.dq.header, CplPropertyList)


# ---------- shape check ----------


class TestEnhancedImageShapeCheck:
    def test_mismatched_error_raises(self):
        with pytest.raises(ValueError, match='error'):
            EnhancedImage(make_image(4, 4), make_image(5, 6), prefix=PREFIX)

    def test_mismatched_dq_raises(self):
        with pytest.raises(ValueError, match='dq'):
            EnhancedImage(make_image(4, 4), dq=build_dq_mask(5, 6), prefix=PREFIX)

    def test_matching_dimensions_pass(self):
        # Should not raise.
        EnhancedImage(make_image(4, 6), make_image(4, 6), build_dq_mask(4, 6), prefix=PREFIX)

    def test_imagelist_with_matching_plane_dims_ok(self):
        """
        The lingain case: a coefficient stack (ImageList) paired with a single
        2D bad-pixel map. The plane count differs but the spatial dims match,
        so construction must succeed.
        """
        ei = EnhancedImage(make_imagelist(3, 4, 4),
                           make_imagelist(3, 4, 4),
                           build_dq_mask(4, 4),
                           prefix=PREFIX)
        assert isinstance(ei.image.data, CplImageList)
        assert isinstance(ei.dq.data, CplImage)

    def test_imagelist_with_mismatched_plane_dims_raises(self):
        with pytest.raises(ValueError, match='dq'):
            EnhancedImage(make_imagelist(3, 4, 4), dq=build_dq_mask(5, 6), prefix=PREFIX)


# ---------- _dimensions helper ----------


class TestDimensions:
    def test_image_dimensions(self):
        img = make_image(4, 6)
        assert EnhancedImage._dimensions(img) == (img.width, img.height)

    def test_imagelist_uses_first_plane(self):
        img = make_image(4, 6)
        ilist = CplImageList([img])
        assert EnhancedImage._dimensions(ilist) == EnhancedImage._dimensions(img)

    def test_empty_imagelist_raises(self):
        with pytest.raises(ValueError, match='empty ImageList'):
            EnhancedImage._dimensions(CplImageList())


# ---------- as_list / repr ----------


class TestAsListAndRepr:
    def test_as_list_all_layers(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        hdus = ei.as_list()
        assert len(hdus) == 3
        assert all(isinstance(h, Hdu) for h in hdus)

    def test_as_list_skips_absent_layers(self):
        """With no error layer, as_list() must not contain a None placeholder."""
        ei = EnhancedImage(make_image(), dq=build_dq_mask(), prefix=PREFIX)
        hdus = ei.as_list()
        assert len(hdus) == 2
        assert None not in hdus
        assert {h.name for h in hdus} == {f'{PREFIX}.SCI', f'{PREFIX}.DQ'}

    def test_as_list_image_only(self):
        ei = EnhancedImage(make_image(), prefix=PREFIX)
        assert ei.as_list() == [ei.image]

    def test_repr_contains_prefix_and_layer_names(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        text = repr(ei)
        assert PREFIX in text
        assert f'{PREFIX}.SCI' in text
        assert f'{PREFIX}.DQ' in text

    def test_repr_describes_image_size_and_type(self):
        ei = EnhancedImage(make_image(4, 6), prefix=PREFIX)
        # width × height reported once; make_image(rows=4, cols=6) -> 6 wide, 4 tall.
        assert f'{PREFIX}.SCI: Image' in repr(ei)
        assert '6×4' in repr(ei)

    def test_repr_describes_imagelist_depth(self):
        ei = EnhancedImage(make_imagelist(3, 4, 6), prefix=PREFIX)
        assert f'{PREFIX}.SCI: ImageList[3]' in repr(ei)

    def test_repr_reports_dimensions_once(self):
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), build_dq_mask(4, 6), prefix=PREFIX)
        # All layers share dimensions, so the size appears exactly once.
        assert repr(ei).count('6×4') == 1


# ---------- save / load round-trip ----------


def seed_primary(filename: str) -> None:
    """Create a FITS file with an (empty) primary HDU for extensions to append to."""
    CplPropertyList().save(filename, cpl.core.io.CREATE)


class TestSaveLoadRoundTrip:
    def test_round_trip_all_layers(self, tmp_path):
        filename = str(tmp_path / 'enhanced.fits')
        sci = make_image(4, 6, value=3.5)
        err = make_image(4, 6, value=0.1)
        dq = build_dq_mask(4, 6)

        seed_primary(filename)
        EnhancedImage(sci, err, dq, prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)

        assert loaded.image.name == f'{PREFIX}.SCI'
        assert loaded.error is not None
        assert loaded.dq is not None

        np.testing.assert_allclose(loaded.image.data.as_array(), sci.as_array())
        np.testing.assert_allclose(loaded.error.data.as_array(), err.as_array())
        np.testing.assert_array_equal(loaded.dq.data.as_array(), dq.as_array())

    def test_dq_loads_as_integer(self, tmp_path):
        """The data quality mask must survive the round-trip as an integer type."""
        filename = str(tmp_path / 'dq.fits')
        seed_primary(filename)
        EnhancedImage(make_image(), dq=build_dq_mask(), prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)
        assert np.issubdtype(loaded.dq.data.as_array().dtype, np.integer)

    def test_round_trip_image_only(self, tmp_path):
        filename = str(tmp_path / 'sci_only.fits')
        seed_primary(filename)
        EnhancedImage(make_image(), prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)
        assert loaded.error is None
        assert loaded.dq is None

    def test_round_trip_imagelist(self, tmp_path):
        """A NAXIS == 3 extension must come back as an ImageList of the same depth."""
        filename = str(tmp_path / 'stack.fits')
        stack = make_imagelist(3, 4, 4)
        seed_primary(filename)
        EnhancedImage(stack, prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)
        assert isinstance(loaded.image.data, CplImageList)
        assert len(loaded.image.data) == len(stack)

    def test_load_missing_sci_raises(self, tmp_path):
        filename = str(tmp_path / 'empty.fits')
        seed_primary(filename)
        with pytest.raises(cpl.core.DataNotFoundError):
            EnhancedImage.load(filename, PREFIX)
