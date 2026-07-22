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
from cpl.core import Mask as CplMask

from pymetis.engine.core.classes.mask import Mask


def cpl_mask(pattern: list[list[bool]]) -> CplMask:
    """A 1-bit `cpl.core.Mask` from a nested list of booleans (rows of columns)."""
    return CplMask(np.array(pattern, dtype=bool))


def make_mask(pattern: list[list[int]]) -> Mask:
    """A `Mask` whose bitfield is given directly as a nested list of ints."""
    array = np.array(pattern, dtype=np.int32)
    return Mask(cpl.core.Image(array, dtype=cpl.core.Type.INT))


# A 2×2 selection with the top-left pixel set, reused throughout.
TOP_LEFT = [[True, False], [False, False]]
BOTTOM_ROW = [[False, False], [True, True]]


# ---------- converting constructor ----------


class TestConstructor:
    def test_from_cpl_image_copies(self):
        image = cpl.core.Image(np.array([[1, 0], [0, 2]], dtype=np.int32),
                               dtype=cpl.core.Type.INT)
        mask = Mask(image)
        np.testing.assert_array_equal(mask._array(), [[1, 0], [0, 2]])
        # The mask owns its own buffer, not the image passed in.
        assert mask.data is not image

    def test_from_cpl_mask_sets_bit_zero(self):
        mask = Mask(cpl_mask([[True, False], [False, True]]))
        np.testing.assert_array_equal(mask._array(), [[1, 0], [0, 1]])

    def test_from_mask_is_an_independent_copy(self):
        original = make_mask([[1, 2], [4, 8]])
        clone = Mask(original)
        np.testing.assert_array_equal(clone._array(), original._array())

        original.add(cpl_mask(TOP_LEFT), 0x10)
        assert clone._array()[0, 0] == 1  # clone unaffected by mutation of source

    def test_rejects_unsupported_type(self):
        with pytest.raises(TypeError):
            Mask(42)


# ---------- construction from CPL masks ----------


class TestFromCplMask:
    def test_single_bit_sets_that_bit(self):
        mask = Mask.from_cpl_mask(cpl_mask(TOP_LEFT), 0x04)
        np.testing.assert_array_equal(mask._array(), [[4, 0], [0, 0]])

    def test_high_single_bit_accepted(self):
        """A legitimate single bit far from bit 0 (previously rejected by a
        faulty bit_length guard) must be accepted."""
        mask = Mask.from_cpl_mask(cpl_mask(TOP_LEFT), 1 << 20)
        assert mask._array()[0, 0] == (1 << 20)

    def test_multi_bit_value_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            Mask.from_cpl_mask(cpl_mask(TOP_LEFT), 0x03)

    def test_zero_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            Mask.from_cpl_mask(cpl_mask(TOP_LEFT), 0)


class TestFromCplMasks:
    def test_combines_disjoint_bits(self):
        mask = Mask.from_cpl_masks({0x01: cpl_mask(TOP_LEFT),
                                    0x04: cpl_mask(BOTTOM_ROW)})
        np.testing.assert_array_equal(mask._array(), [[1, 0], [4, 4]])

    def test_overlapping_pixels_or_their_bits(self):
        """A pixel flagged by two masks carries both bits."""
        mask = Mask.from_cpl_masks({0x01: cpl_mask(TOP_LEFT),
                                    0x02: cpl_mask(TOP_LEFT)})
        assert mask._array()[0, 0] == 0x03

    def test_unset_pixels_are_zero(self):
        """Unflagged pixels must be a clean zero, not uninitialised memory."""
        mask = Mask.from_cpl_masks({0x01: cpl_mask(TOP_LEFT)})
        cleared = mask._array()
        cleared[0, 0] = 0
        assert not cleared.any()

    def test_stored_as_signed_int32(self):
        mask = Mask.from_cpl_masks({0x01: cpl_mask(TOP_LEFT)})
        assert mask._array().dtype == np.int32

    def test_mismatched_shapes_raise(self):
        small = cpl_mask([[True, False], [False, False]])
        big = cpl_mask([[True, False, False], [False, False, False]])
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='same width and height'):
            Mask.from_cpl_masks({0x01: small, 0x02: big})

    def test_empty_dict_raises(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='same width and height'):
            Mask.from_cpl_masks({})


# ---------- flatten ----------


class TestFlatten:
    def test_any_nonzero_bit_is_bad(self):
        mask = make_mask([[0, 1], [4, 0]])
        np.testing.assert_array_equal(np.asarray(mask.flatten()),
                                      [[False, True], [True, False]])

    def test_multi_bit_pixel_is_bad(self):
        """A pixel with several flags set collapses to a single bad pixel."""
        mask = make_mask([[0x05, 0], [0, 0]])
        assert np.asarray(mask.flatten())[0, 0]

    def test_all_zero_is_all_good(self):
        mask = make_mask([[0, 0], [0, 0]])
        assert not np.asarray(mask.flatten()).any()

    def test_returns_boolean_cpl_mask(self):
        result = make_mask([[1, 0], [0, 0]]).flatten()
        assert isinstance(result, CplMask)
        assert np.asarray(result).dtype == np.bool_


# ---------- combination operators ----------


class TestOperators:
    def test_or_unions_flags(self):
        a = make_mask([[0x01, 0x00], [0, 0]])
        b = make_mask([[0x02, 0x04], [0, 0]])
        np.testing.assert_array_equal((a | b)._array(), [[0x03, 0x04], [0, 0]])

    def test_and_intersects_flags(self):
        a = make_mask([[0x03, 0x04], [0, 0]])
        b = make_mask([[0x01, 0x04], [0, 0]])
        np.testing.assert_array_equal((a & b)._array(), [[0x01, 0x04], [0, 0]])

    def test_operators_return_new_mask(self):
        a = make_mask([[0x01, 0], [0, 0]])
        b = make_mask([[0x02, 0], [0, 0]])
        combined = a | b
        assert isinstance(combined, Mask)
        # Operands are left untouched.
        assert a._array()[0, 0] == 0x01
        assert b._array()[0, 0] == 0x02


# ---------- add (in place) ----------


class TestAdd:
    def test_sets_bit_where_selected(self):
        mask = make_mask([[0, 0], [0, 0]])
        mask.add(cpl_mask(BOTTOM_ROW), 0x04)
        np.testing.assert_array_equal(mask._array(), [[0, 0], [4, 4]])

    def test_preserves_existing_bits(self):
        mask = make_mask([[0x01, 0], [0, 0]])
        mask.add(cpl_mask(TOP_LEFT), 0x02)
        assert mask._array()[0, 0] == 0x03

    def test_is_idempotent_for_same_bit(self):
        mask = make_mask([[0x04, 0], [0, 0]])
        mask.add(cpl_mask(TOP_LEFT), 0x04)
        assert mask._array()[0, 0] == 0x04

    def test_returns_self_for_chaining(self):
        mask = make_mask([[0, 0], [0, 0]])
        assert mask.add(cpl_mask(TOP_LEFT), 0x01) is mask


# ---------- single-bit extraction ----------


class TestGetItem:
    def test_isolates_requested_bit(self):
        mask = make_mask([[0x05, 0x04], [0x01, 0x00]])
        # Bit 0x04 is set in the two pixels that contain it, regardless of
        # whatever other bits (0x01) those pixels also carry.
        np.testing.assert_array_equal(np.asarray(mask[0x04]),
                                      [[True, True], [False, False]])

    def test_returns_boolean_cpl_mask(self):
        result = make_mask([[0x01, 0], [0, 0]])[0x01]
        assert isinstance(result, CplMask)
        assert np.asarray(result).dtype == np.bool_

    def test_multi_bit_value_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            _ = make_mask([[0x03, 0], [0, 0]])[0x03]

    def test_zero_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            _ = make_mask([[0x01, 0], [0, 0]])[0]


# ---------- sign-bit constraint ----------


class TestFlagBitBound:
    def test_max_flag_bit_leaves_sign_bit_free(self):
        """Only bits 0-30 are usable; bit 31 is the sign bit of the int32 store."""
        assert Mask.MAX_FLAG_BIT == 30

    def test_highest_allowed_bit_stays_non_negative(self):
        mask = Mask.from_cpl_mask(cpl_mask(TOP_LEFT), 1 << Mask.MAX_FLAG_BIT)
        assert mask._array()[0, 0] > 0
