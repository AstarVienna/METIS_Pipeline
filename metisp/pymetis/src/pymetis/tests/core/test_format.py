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

from pymetis.core.format import partial_format


class TestFormat:
    def test_partial_format_basic(self):
        assert partial_format("METIS_{det}_DARK", det='2RG') == 'METIS_2RG_DARK'

    def test_partial_format_partial(self):
        assert partial_format("{det}_BLAH_{undef}", det='GEO') == 'GEO_BLAH_{undef}'

    def test_partial_format_double(self):
        assert partial_format("{foo}_{bar}", foo='qux', bar='baz') == 'qux_baz'

    def test_partial_format_undefined(self):
        assert partial_format("{foo}_{bar}", foo='baz', baz='foo') == 'baz_{bar}'

    def test_partial_format_complex(self):
        assert partial_format("{foo}_ELT_{baz}", foo='FOO', bar='BAR') == 'FOO_ELT_{baz}'