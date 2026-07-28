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
                      Mask as CplMask,
                      PropertyList as CplPropertyList)
from cpl.hdrl.core import Image as HdrlImage

from pymetis.engine.core.classes.image import EnhancedImage
from pymetis.engine.core.classes.mask import DataQuality
from pymetis.engine.dataitems import Hdu


PREFIX = 'DET1'


def make_image(rows: int = 4, cols: int = 4, value: float = 1.0) -> CplImage:
    """A float `Image` of the given shape filled with `value`."""
    return CplImage(data=np.full((rows, cols), value, dtype=np.float64))


def build_dq_mask(rows: int = 4, cols: int = 4) -> DataQuality:
    """A 32-bit `Mask` with a couple of flagged pixels."""
    bits = np.zeros((rows, cols), dtype=np.int32)
    bits[0, 0] = 1
    bits[1, 2] = 1
    return DataQuality(CplImage(bits, dtype=cpl.core.Type.INT))


def make_imagelist(planes: int = 3, rows: int = 4, cols: int = 4) -> CplImageList:
    """An `ImageList` of `planes` equally-sized float images."""
    return CplImageList([make_image(rows, cols, float(i)) for i in range(planes)])


def seed_primary(filename: str) -> None:
    """Create a FITS file with an (empty) primary HDU for extensions to append to."""
    CplPropertyList().save(filename, cpl.core.io.CREATE)


# ---------- construction ----------


class TestConstruction:
    def test_science_and_error_wrapped_as_hdrl_image(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        assert isinstance(ei.image, HdrlImage)

    def test_dq_wrapped_as_mask(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        assert isinstance(ei.dq, DataQuality)
        np.testing.assert_array_equal(ei.dq._array(), build_dq_mask()._array())

    def test_dq_mask_is_copied_not_shared(self):
        """The image must not alias the caller's mask: mutating the source
        afterwards leaves the stored data quality layer untouched."""
        source = build_dq_mask(4, 6)
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), source, prefix=PREFIX)
        before = ei.dq._array().copy()

        source.add(CplMask(np.ones((4, 6), dtype=bool)), 0x10)

        np.testing.assert_array_equal(ei.dq._array(), before)

    def test_cpl_mask_dq_sets_bit_zero(self):
        """A 1-bit CplMask is promoted into the 32-bit mask as bit 0."""
        bad = np.zeros((4, 6), dtype=bool)
        bad[0, 0] = True
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), CplMask(bad), prefix=PREFIX)
        assert isinstance(ei.dq, DataQuality)
        np.testing.assert_array_equal(ei.dq._array(), bad.astype(np.int32))

    def test_cpl_image_dq_accepted(self):
        """A raw integer CplImage is implicitly converted into a Mask."""
        dq_image = build_dq_mask(4, 6).data  # the underlying CplImage
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), dq_image, prefix=PREFIX)
        assert isinstance(ei.dq, DataQuality)
        np.testing.assert_array_equal(ei.dq._array(), build_dq_mask(4, 6)._array())

    def test_prefix_stored(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        assert ei.prefix == PREFIX

    def test_missing_headers_default_to_empty(self):
        """Headers are optional; omitting them must yield empty property lists."""
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        assert isinstance(ei.header_image, CplPropertyList)
        assert isinstance(ei.header_error, CplPropertyList)
        assert isinstance(ei.header_dq, CplPropertyList)

    def test_supplied_headers_preserved(self):
        header = CplPropertyList()
        header.append(cpl.core.Property('HIERARCH ESO FOO', 42))
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(),
                           prefix=PREFIX, header_image=header)
        assert ei.header_image['HIERARCH ESO FOO'].value == 42

    def test_absent_dq_defaults_to_zero_mask(self):
        """Omitting the data quality layer yields an all-good integer mask of
        the image's shape, not a broken `Mask`."""
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), prefix=PREFIX)
        assert isinstance(ei.dq, DataQuality)
        dq = ei.dq.data.as_array()
        assert not dq.any()
        assert dq.shape == (4, 6)
        assert np.issubdtype(dq.dtype, np.integer)


# ---------- from_hdrl pseudo-constructor ----------


class TestFromHdrl:
    def test_unpacks_science_and_error(self):
        hdrl = HdrlImage(make_image(4, 6, value=2.0), make_image(4, 6, value=0.5))
        ei = EnhancedImage.from_hdrl(hdrl, build_dq_mask(4, 6), prefix=PREFIX)

        assert isinstance(ei.image, HdrlImage)
        np.testing.assert_allclose(ei.image.image.as_array(), np.full((4, 6), 2.0))
        np.testing.assert_allclose(ei.image.error.as_array(), np.full((4, 6), 0.5))

    def test_absent_dq_defaults_to_zero_mask(self):
        hdrl = HdrlImage(make_image(4, 6), make_image(4, 6))
        ei = EnhancedImage.from_hdrl(hdrl, prefix=PREFIX)
        assert isinstance(ei.dq, DataQuality)
        assert not ei.dq.data.as_array().any()


# ---------- validation ----------


class TestValidation:
    def test_mismatched_error_dimensions_raise(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='error'):
            EnhancedImage(make_image(4, 4), make_image(5, 6), build_dq_mask(4, 4),
                          prefix=PREFIX)

    def test_mismatched_dq_dimensions_raise(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='dq'):
            EnhancedImage(make_image(4, 4), make_image(4, 4), build_dq_mask(5, 6),
                          prefix=PREFIX)

    def test_matching_dimensions_pass(self):
        # Should not raise.
        EnhancedImage(make_image(4, 6), make_image(4, 6), build_dq_mask(4, 6), prefix=PREFIX)


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


# ---------- _zeros_like helper ----------


class TestZerosLike:
    def test_zeros_like_image_matches_shape(self):
        zeros = EnhancedImage._zeros_like(make_image(4, 6, value=9.0))
        assert isinstance(zeros, CplImage)
        assert (zeros.width, zeros.height) == (6, 4)
        assert not zeros.as_array().any()

    def test_zeros_like_imagelist_matches_shape_and_depth(self):
        zeros = EnhancedImage._zeros_like(make_imagelist(3, 4, 6))
        assert isinstance(zeros, CplImageList)
        assert len(zeros) == 3
        assert (zeros[0].width, zeros[0].height) == (6, 4)
        assert not zeros[0].as_array().any()


# ---------- _describe_layer / repr ----------


class TestDescribeLayerAndRepr:
    def test_describe_image(self):
        assert EnhancedImage._describe_layer('DET1.SCI', make_image()) == 'DET1.SCI: Image'

    def test_describe_imagelist_reports_depth(self):
        assert EnhancedImage._describe_layer('DET1.SCI', make_imagelist(3)) == 'DET1.SCI: ImageList[3]'

    def test_repr_contains_prefix_and_layer_names(self):
        ei = EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX)
        text = repr(ei)
        assert PREFIX in text
        assert f'{PREFIX}.SCI' in text
        assert f'{PREFIX}.ERR' in text
        assert f'{PREFIX}.DQ' in text

    def test_repr_reports_dimensions_once(self):
        ei = EnhancedImage(make_image(4, 6), make_image(4, 6), build_dq_mask(4, 6), prefix=PREFIX)
        # make_image(rows=4, cols=6) -> 6 wide, 4 tall; shared across layers.
        assert repr(ei).count('6×4') == 1


# ---------- save / load round-trip ----------


class TestSaveLoadRoundTrip:
    def test_round_trip_all_layers(self, tmp_path):
        filename = str(tmp_path / 'enhanced.fits')
        sci = make_image(4, 6, value=3.5)
        err = make_image(4, 6, value=0.1)
        dq = build_dq_mask(4, 6)

        seed_primary(filename)
        EnhancedImage(sci, err, dq, prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)

        np.testing.assert_allclose(loaded.image.image.as_array(), sci.as_array())
        np.testing.assert_allclose(loaded.image.error.as_array(), err.as_array())
        np.testing.assert_array_equal(loaded.dq.data.as_array(), dq._array())

    def test_dq_loads_as_integer(self, tmp_path):
        """The data quality mask must survive the round-trip as an integer type."""
        filename = str(tmp_path / 'dq.fits')
        seed_primary(filename)
        EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)
        assert np.issubdtype(loaded.dq.data.as_array().dtype, np.integer)

    def test_absent_error_and_dq_default_to_zeros(self, tmp_path):
        """A file holding only the SCI extension must load with zero-filled
        error and data quality layers of the matching shape."""
        filename = str(tmp_path / 'sci_only.fits')
        seed_primary(filename)
        # Write only the science extension, bypassing EnhancedImage.save.
        Hdu(CplPropertyList(), make_image(4, 6, value=7.0),
            name=f'{PREFIX}.{EnhancedImage.sci_suffix}').save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)

        assert not loaded.image.error.as_array().any()
        dq = loaded.dq.data.as_array()
        assert not dq.any()
        assert dq.shape == (4, 6)
        assert np.issubdtype(dq.dtype, np.integer)

    def test_load_recovers_extname_into_headers(self, tmp_path):
        filename = str(tmp_path / 'headers.fits')
        seed_primary(filename)
        EnhancedImage(make_image(), make_image(), build_dq_mask(), prefix=PREFIX).save(filename)

        loaded = EnhancedImage.load(filename, PREFIX)
        assert loaded.header_image['EXTNAME'].value == f'{PREFIX}.{EnhancedImage.sci_suffix}'
        assert loaded.header_dq['EXTNAME'].value == f'{PREFIX}.{EnhancedImage.dq_suffix}'

    def test_load_missing_sci_raises(self, tmp_path):
        filename = str(tmp_path / 'empty.fits')
        seed_primary(filename)
        with pytest.raises(cpl.core.DataNotFoundError):
            EnhancedImage.load(filename, PREFIX)

