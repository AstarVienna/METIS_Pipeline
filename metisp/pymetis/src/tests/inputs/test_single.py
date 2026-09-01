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

from pymetis.instruments.metis.inputs.common import (PersistenceMapInput,
                                                     OptionalPersistenceMapInput)


@pytest.fixture
def absent_optional():
    """An optional input whose frame was never matched (`frame` stays None)."""
    return OptionalPersistenceMapInput(cpl.ui.FrameSet())


@pytest.fixture
def absent_required():
    """A required input whose frame was never matched."""
    return PersistenceMapInput(cpl.ui.FrameSet())


class TestOptionalInputAbsent:
    """
    An optional input with no matching frame must be skipped, not crash.

    `OptionalInputMixin` only sets `_required = False`, which governs
    verification. Before this was fixed, the loading path dereferenced
    `self.frame` unconditionally and raised
    `AttributeError: 'NoneType' object has no attribute 'file'`
    as soon as a recipe was handed a frameset without the optional frame.
    """

    def test_frame_is_none(self, absent_optional):
        assert absent_optional.frame is None

    def test_validate_passes(self, absent_optional):
        absent_optional.validate()

    def test_load_structure_does_not_raise(self, absent_optional):
        absent_optional.load_structure()
        assert absent_optional.item is None

    def test_load_data_returns_none(self, absent_optional):
        assert absent_optional.load_data() is None

    def test_load_data_with_extension_returns_none(self, absent_optional):
        assert absent_optional.load_data('SOME_EXTENSION') is None

    def test_set_cpl_attributes_does_not_raise(self, absent_optional):
        absent_optional.set_cpl_attributes()

    def test_use_does_not_raise(self, absent_optional):
        assert absent_optional.use() is absent_optional

    def test_valid_frames_is_empty(self, absent_optional):
        assert len(absent_optional.valid_frames()) == 0

    def test_used_frames_is_empty(self, absent_optional):
        assert len(absent_optional.used_frames()) == 0


class TestRequiredInputAbsent:
    """
    A required input with no matching frame must still fail, and fail loudly.

    Skipping the load for optional inputs must not silently swallow a genuinely
    missing required one -- that would turn a clear `DataNotFoundError` into a
    confusing downstream failure.
    """

    def test_frame_is_none(self, absent_required):
        assert absent_required.frame is None

    def test_validate_raises(self, absent_required):
        with pytest.raises(cpl.core.DataNotFoundError):
            absent_required.validate()

    def test_load_structure_raises(self, absent_required):
        with pytest.raises(cpl.core.DataNotFoundError):
            absent_required.load_structure()

    def test_load_data_raises(self, absent_required):
        with pytest.raises(cpl.core.DataNotFoundError):
            absent_required.load_data()
