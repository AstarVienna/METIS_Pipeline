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
import cpl.hdrl.core
from cpl.core import Mask as CplMask

from pymetis.engine.core.classes.mask import DataQuality


# Masks are exercised at a realistic-ish detector scale rather than a toy 2×2,
# with a few scattered pixels standing in for flagged defects.
SIZE = 8
A = (0, 0)
B = (3, 5)
C = (7, 7)


def selection(*coords: tuple[int, int]) -> np.ndarray:
    """A boolean ``SIZE × SIZE`` array with the given (row, col) pixels set True."""
    array = np.zeros((SIZE, SIZE), dtype=bool)
    for row, col in coords:
        array[row, col] = True
    return array


def bitfield(pixels: dict[tuple[int, int], int]) -> np.ndarray:
    """An int32 ``SIZE × SIZE`` bitfield with ``{(row, col): value}``; rest zero."""
    array = np.zeros((SIZE, SIZE), dtype=np.int32)
    for (row, col), value in pixels.items():
        array[row, col] = value
    return array


def cpl_mask(selected: np.ndarray) -> CplMask:
    """A 1-bit `cpl.core.Mask` from a boolean numpy array."""
    return CplMask(selected)


def make_mask(field: np.ndarray) -> DataQuality:
    """A `Mask` wrapping the given int32 numpy bitfield."""
    return DataQuality(cpl.core.Image(field.astype(np.int32), dtype=cpl.core.Type.INT))


# ---------- converting constructor ----------


class TestConstructor:
    def test_from_cpl_image_copies(self):
        field = bitfield({A: 1, B: 2})
        image = cpl.core.Image(field, dtype=cpl.core.Type.INT)
        mask = DataQuality(image)
        np.testing.assert_array_equal(mask._array(), field)
        # The mask owns its own buffer, not the image passed in.
        assert mask.data is not image

    def test_from_cpl_mask_sets_bit_zero(self):
        selected = selection(A, C)
        mask = DataQuality(cpl_mask(selected))
        np.testing.assert_array_equal(mask._array(), selected.astype(np.int32))

    def test_from_mask_is_an_independent_copy(self):
        original = make_mask(bitfield({A: 1, B: 2, C: 4}))
        clone = DataQuality(original)
        np.testing.assert_array_equal(clone._array(), original._array())

        original.add(cpl_mask(selection(A)), 0x10)
        assert clone._array()[A] == 1  # clone unaffected by mutation of source

    def test_rejects_unsupported_type(self):
        with pytest.raises(TypeError):
            DataQuality(42)


# ---------- construction from CPL masks ----------


class TestFromCplMask:
    def test_single_bit_sets_that_bit(self):
        mask = DataQuality.from_cpl_mask(cpl_mask(selection(A)), 0x04)
        np.testing.assert_array_equal(mask._array(), bitfield({A: 0x04}))

    def test_high_single_bit_accepted(self):
        """A legitimate single bit far from bit 0 (previously rejected by a
        faulty bit_length guard) must be accepted."""
        mask = DataQuality.from_cpl_mask(cpl_mask(selection(A)), 1 << 20)
        assert mask._array()[A] == (1 << 20)

    def test_multi_bit_value_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            DataQuality.from_cpl_mask(cpl_mask(selection(A)), 0x03)

    def test_zero_rejected(self):
        with pytest.raises(ValueError, match='Only one bit'):
            DataQuality.from_cpl_mask(cpl_mask(selection(A)), 0)


class TestFromCplMasks:
    def test_combines_disjoint_bits(self):
        mask = DataQuality.from_cpl_masks({0x01: cpl_mask(selection(A)),
                                           0x04: cpl_mask(selection(B, C))})
        np.testing.assert_array_equal(mask._array(), bitfield({A: 0x01, B: 0x04, C: 0x04}))

    def test_overlapping_pixels_or_their_bits(self):
        """A pixel flagged by two masks carries both bits."""
        mask = DataQuality.from_cpl_masks({0x01: cpl_mask(selection(A)),
                                           0x02: cpl_mask(selection(A))})
        assert mask._array()[A] == 0x03

    def test_unset_pixels_are_zero(self):
        """Unflagged pixels must be a clean zero, not uninitialised memory."""
        mask = DataQuality.from_cpl_masks({0x01: cpl_mask(selection(A))})
        cleared = mask._array()
        cleared[A] = 0
        assert not cleared.any()

    def test_stored_as_signed_int32(self):
        mask = DataQuality.from_cpl_masks({0x01: cpl_mask(selection(A))})
        assert mask._array().dtype == np.int32

    def test_mismatched_shapes_raise(self):
        small = CplMask(np.zeros((SIZE, SIZE), dtype=bool))
        big = CplMask(np.zeros((SIZE, SIZE + 2), dtype=bool))
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='same width and height'):
            DataQuality.from_cpl_masks({0x01: small, 0x02: big})

    def test_empty_dict_raises(self):
        with pytest.raises(cpl.hdrl.core.IncompatibleInputError, match='same width and height'):
            DataQuality.from_cpl_masks({})


# ---------- flatten ----------


class TestFlatten:
    def test_any_nonzero_bit_is_bad(self):
        mask = make_mask(bitfield({B: 0x01, C: 0x04}))
        np.testing.assert_array_equal(np.asarray(mask.flatten()), selection(B, C))

    def test_multi_bit_pixel_is_bad(self):
        """A pixel with several flags set collapses to a single bad pixel."""
        mask = make_mask(bitfield({A: 0x05}))
        assert np.asarray(mask.flatten())[A]

    def test_all_zero_is_all_good(self):
        mask = make_mask(bitfield({}))
        assert not np.asarray(mask.flatten()).any()

    def test_returns_boolean_cpl_mask(self):
        result = make_mask(bitfield({A: 0x01})).flatten()
        assert isinstance(result, CplMask)
        assert np.asarray(result).dtype == np.bool_


# ---------- combination operators ----------


class TestOperators:
    def test_or_unions_flags(self):
        a = make_mask(bitfield({A: 0x01}))
        b = make_mask(bitfield({A: 0x02, B: 0x04}))
        np.testing.assert_array_equal((a | b)._array(), bitfield({A: 0x03, B: 0x04}))

    def test_and_intersects_flags(self):
        a = make_mask(bitfield({A: 0x03, B: 0x04}))
        b = make_mask(bitfield({A: 0x01, B: 0x04}))
        np.testing.assert_array_equal((a & b)._array(), bitfield({A: 0x01, B: 0x04}))

    def test_operators_return_new_mask(self):
        a = make_mask(bitfield({A: 0x01}))
        b = make_mask(bitfield({A: 0x02}))
        combined = a | b
        assert isinstance(combined, DataQuality)
        # Operands are left untouched.
        assert a._array()[A] == 0x01
        assert b._array()[A] == 0x02


# ---------- add (in place) ----------


class TestAdd:
    def test_sets_bit_where_selected(self):
        mask = make_mask(bitfield({}))
        mask.add(cpl_mask(selection(B, C)), 0x04)
        np.testing.assert_array_equal(mask._array(), bitfield({B: 0x04, C: 0x04}))

    def test_preserves_existing_bits(self):
        mask = make_mask(bitfield({A: 0x01}))
        mask.add(cpl_mask(selection(A)), 0x02)
        assert mask._array()[A] == 0x03

    def test_is_idempotent_for_same_bit(self):
        mask = make_mask(bitfield({A: 0x04}))
        mask.add(cpl_mask(selection(A)), 0x04)
        assert mask._array()[A] == 0x04

    def test_returns_self_for_chaining(self):
        mask = make_mask(bitfield({}))
        assert mask.add(cpl_mask(selection(A)), 0x01) is mask


# ---------- single-bit extraction ----------


class TestGetItem:
    def test_isolates_requested_bit(self):
        # A and B carry bit 0x04 (A alongside 0x01); C carries only 0x01.
        mask = make_mask(bitfield({A: 0x05, B: 0x04, C: 0x01}))
        np.testing.assert_array_equal(np.asarray(mask[0x04]), selection(A, B))

    def test_matches_any_of_a_combined_flag(self):
        """Indexing with several OR-ed bits selects pixels carrying any of them."""
        mask = make_mask(bitfield({A: 0x01, B: 0x02, C: 0x04}))
        np.testing.assert_array_equal(np.asarray(mask[0x01 | 0x02]), selection(A, B))

    def test_zero_selects_nothing(self):
        mask = make_mask(bitfield({A: 0x01, C: 0x04}))
        assert not np.asarray(mask[0]).any()

    def test_returns_boolean_cpl_mask(self):
        result = make_mask(bitfield({A: 0x01}))[0x01]
        assert isinstance(result, CplMask)
        assert np.asarray(result).dtype == np.bool_


# ---------- sign-bit constraint ----------


class TestSignedInt32Storage:
    """int32 storage detail: bit 30 is the highest flag bit that stays
    non-negative; bit 31 is the sign bit. The usable-bit bound itself now
    lives on ``InstrumentDescription.MaskFlags.MAX_FLAG_BIT``."""

    def test_bit_30_stays_non_negative(self):
        mask = DataQuality.from_cpl_mask(cpl_mask(selection(A)), 1 << 30)
        value = mask._array()[A]
        assert value == (1 << 30)
        assert value > 0
