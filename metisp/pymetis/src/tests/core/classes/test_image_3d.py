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
                      PropertyList as CplPropertyList,
                      Type as CplType)
from cpl.hdrl.core import (ImageList as HdrlImageList)

from pymetis.engine.core.classes.image import (EnhancedImage,
                                               EnhancedImage3D,
                                               EnhancedImageBase)
from pymetis.engine.core.classes.dataquality import DataQuality
from pymetis.engine.dataitems import Hdu


PREFIX = 'DET1'
ROWS, COLS = 4, 6


def stack(planes: int = 3, rows: int = ROWS, cols: int = COLS, base: float = 0.0) -> CplImageList:
    """An `ImageList` of `planes` frames; plane i is filled with base + i."""
    return CplImageList([CplImage(np.full((rows, cols), base + i, dtype=np.float64))
                         for i in range(planes)])


def build_dq_mask(rows: int = ROWS, cols: int = COLS) -> DataQuality:
    """A 32-bit `Mask` with a couple of flagged pixels."""
    bits = np.zeros((rows, cols), dtype=np.int32)
    bits[0, 0] = 1
    bits[1, 2] = 1
    return DataQuality(CplImage(bits, dtype=CplType.INT))


def seed_primary(filename: str) -> None:
    CplPropertyList().save(filename, cpl.core.io.CREATE)


# ---------- construction ----------


class TestConstruction:
    def test_science_and_error_wrapped_as_hdrl_image_list(self):
        eil = EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX)
        assert isinstance(eil.image, HdrlImageList)
        assert len(eil.image) == 3

    def test_dq_is_a_single_2d_mask(self):
        eil = EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX)
        assert isinstance(eil.dq, DataQuality)
        assert eil.dq.data.as_array().shape == (ROWS, COLS)
        np.testing.assert_array_equal(eil.dq._array(), build_dq_mask()._array())

    def test_preserves_stack_depth(self):
        eil = EnhancedImage3D(stack(5), stack(5), prefix=PREFIX)
        assert len(eil._sci_data()) == 5
        assert len(eil._err_data()) == 5

    def test_absent_error_defaults_to_zero_stack(self):
        eil = EnhancedImage3D(stack(3, base=7.0), prefix=PREFIX)
        errors = eil._err_data()
        assert isinstance(errors, CplImageList)
        assert len(errors) == 3
        assert all(not errors[i].as_array().any() for i in range(3))

    def test_absent_dq_defaults_to_zero_mask(self):
        eil = EnhancedImage3D(stack(3), stack(3), prefix=PREFIX)
        assert isinstance(eil.dq, DataQuality)
        dq = eil.dq.data.as_array()
        assert not dq.any()
        assert dq.shape == (ROWS, COLS)


# ---------- validation ----------


class TestValidation:
    def test_non_imagelist_rejected(self):
        with pytest.raises(ValueError, match='Unsupported image type'):
            EnhancedImage3D(CplImage(np.zeros((ROWS, COLS))), prefix=PREFIX)

    def test_empty_imagelist_rejected(self):
        with pytest.raises(ValueError, match='empty ImageList'):
            EnhancedImage3D(CplImageList(), prefix=PREFIX)

    def test_mismatched_error_dimensions_raise(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='error'):
            EnhancedImage3D(stack(3, ROWS, COLS), stack(3, ROWS + 1, COLS), prefix=PREFIX)

    def test_mismatched_error_depth_raises(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='depth'):
            EnhancedImage3D(stack(3), stack(2), prefix=PREFIX)

    def test_mismatched_dq_dimensions_raise(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='dq'):
            EnhancedImage3D(stack(3, ROWS, COLS), stack(3, ROWS, COLS),
                              build_dq_mask(ROWS + 1, COLS), prefix=PREFIX)


# ---------- from_hdrl ----------


class TestFromHdrl:
    def test_unpacks_science_and_error_planes(self):
        hl = HdrlImageList(stack(3, base=2.0), stack(3, base=0.5))
        eil = EnhancedImage3D.from_hdrl(hl, build_dq_mask(), prefix=PREFIX)
        assert isinstance(eil.image, HdrlImageList)
        assert len(eil.image) == 3
        # plane i data == 2 + i, error == 0.5 + i
        np.testing.assert_allclose(eil._sci_data()[2].as_array(), np.full((ROWS, COLS), 4.0))
        np.testing.assert_allclose(eil._err_data()[2].as_array(), np.full((ROWS, COLS), 2.5))


# ---------- schema / repr ----------


class TestSchemaAndRepr:
    def test_schema_uses_imagelist_for_sci_and_err(self):
        schema = EnhancedImage3D.schema(PREFIX)
        assert schema[f'{PREFIX}.SCI'] is CplImageList
        assert schema[f'{PREFIX}.ERR'] is CplImageList
        assert schema[f'{PREFIX}.DQ'] is CplImage   # DQ stays 2D

    def test_repr_reports_depth_and_dimensions(self):
        eil = EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX)
        text = repr(eil)
        assert 'EnhancedImage3D' in text
        assert f'{PREFIX}.SCI: ImageList[3]' in text
        assert f'{PREFIX}.DQ: Image' in text
        assert text.count('6×4') == 1


# ---------- reject ----------


class TestReject:
    def test_reject_pushes_dq_into_every_plane(self):
        eil = EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX)
        eil.reject()
        # build_dq_mask flags 2 pixels; each plane's BPM must pick them up.
        assert all(plane.count_rejected() == 2 for plane in eil.image)


# ---------- save / load round-trip ----------


class TestSaveLoadRoundTrip:
    def test_round_trip_preserves_stack(self, tmp_path):
        filename = str(tmp_path / 'stack.fits')
        sci = stack(3, base=10.0)
        err = stack(3, base=0.1)
        dq = build_dq_mask()

        seed_primary(filename)
        EnhancedImage3D(sci, err, dq, prefix=PREFIX).save(filename)

        loaded = EnhancedImage3D.load(filename, PREFIX)
        assert isinstance(loaded, EnhancedImage3D)
        assert len(loaded._sci_data()) == 3
        np.testing.assert_allclose(loaded._sci_data()[2].as_array(), sci[2].as_array())
        np.testing.assert_allclose(loaded._err_data()[0].as_array(), err[0].as_array())
        np.testing.assert_array_equal(loaded.dq.data.as_array(), dq._array())

    def test_dq_loads_as_integer(self, tmp_path):
        filename = str(tmp_path / 'stack_dq.fits')
        seed_primary(filename)
        EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX).save(filename)
        loaded = EnhancedImage3D.load(filename, PREFIX)
        assert np.issubdtype(loaded.dq.data.as_array().dtype, np.integer)

    def test_absent_error_and_dq_default_to_zeros(self, tmp_path):
        filename = str(tmp_path / 'stack_sci_only.fits')
        seed_primary(filename)
        # Write only the 3D SCI extension.
        Hdu(CplPropertyList(), stack(3, base=7.0),
            name=f'{PREFIX}.{EnhancedImage3D.sci_suffix}').save(filename)

        loaded = EnhancedImage3D.load(filename, PREFIX)
        assert isinstance(loaded, EnhancedImage3D)
        assert len(loaded._err_data()) == 3
        assert all(not loaded._err_data()[i].as_array().any() for i in range(3))
        assert not loaded.dq.data.as_array().any()


# ---------- load auto-dispatch on NAXIS ----------


class TestLoadDispatch:
    def _save_stack(self, filename):
        seed_primary(filename)
        EnhancedImage3D(stack(3), stack(3), build_dq_mask(), prefix=PREFIX).save(filename)

    def _save_single(self, filename):
        seed_primary(filename)
        EnhancedImage(CplImage(np.zeros((ROWS, COLS))),
                      CplImage(np.zeros((ROWS, COLS))),
                      build_dq_mask(), prefix=PREFIX).save(filename)

    def test_3d_file_yields_image_3d_regardless_of_entry_point(self, tmp_path):
        filename = str(tmp_path / 'stack.fits')
        self._save_stack(filename)
        for entry in (EnhancedImageBase, EnhancedImage, EnhancedImage3D):
            loaded = entry.load(filename, PREFIX)
            assert isinstance(loaded, EnhancedImage3D), entry.__name__

    def test_2d_file_yields_single_image_regardless_of_entry_point(self, tmp_path):
        filename = str(tmp_path / 'single.fits')
        self._save_single(filename)
        for entry in (EnhancedImageBase, EnhancedImage, EnhancedImage3D):
            loaded = entry.load(filename, PREFIX)
            assert isinstance(loaded, EnhancedImage), entry.__name__


# ---------- the scratch-pad contract ----------


class TestScratchPadContract:
    def test_rejected_unions_the_planes(self):
        """ Each plane may reject different pixels; `rejected` is their union. """
        ei = EnhancedImage3D(stack(2), prefix=PREFIX)

        first = np.zeros((ROWS, COLS), dtype=bool)
        first[0, 0] = True
        second = np.zeros((ROWS, COLS), dtype=bool)
        second[3, 5] = True
        ei._hdrl_planes()[0].reject_from_mask(CplMask(first))
        ei._hdrl_planes()[1].reject_from_mask(CplMask(second))

        np.testing.assert_array_equal(np.asarray(ei.rejected()), first | second)
