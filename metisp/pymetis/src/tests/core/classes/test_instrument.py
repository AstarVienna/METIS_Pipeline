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

from pymetis.engine.core.classes.instrument import InstrumentDescription
from pymetis.instruments.metis.description import Metis


MaskFlags = InstrumentDescription.MaskFlags


class TestMaskFlagBound:
    """
    The DQ bitfield is stored in a signed int32, so bit 31 is the sign bit and
    must stay unused. `InstrumentDescription.MaskFlags.__init_subclass__`
    enforces that every flag fits within bits 0-`MAX_FLAG_BIT` at the moment a
    subclass is defined.
    """

    def test_max_flag_bit_is_30(self):
        assert MaskFlags.MAX_FLAG_BIT == 30

    def test_highest_allowed_bit_accepted(self):
        # Bit 30 is the last non-sign bit; defining a flag there must succeed.
        class Flags(MaskFlags):
            TOP = 1 << 30

        assert Flags.TOP.value == (1 << 30)

    def test_sign_bit_flag_rejected(self):
        with pytest.raises(ValueError, match='bit above 30'):
            class Flags(MaskFlags):
                SIGN = 1 << 31

    def test_rejection_names_the_offending_member(self):
        with pytest.raises(ValueError, match='SIGN'):
            class Flags(MaskFlags):
                GOOD = 1 << 2
                SIGN = 1 << 31

    def test_real_metis_flags_are_within_bound(self):
        # The production instrument definition must itself pass enforcement.
        assert all(flag.value.bit_length() <= MaskFlags.MAX_FLAG_BIT + 1
                   for flag in Metis.MaskFlags)
        # ...and carry the expected meanings.
        assert Metis.MaskFlags.HOT.value == 0x0004
