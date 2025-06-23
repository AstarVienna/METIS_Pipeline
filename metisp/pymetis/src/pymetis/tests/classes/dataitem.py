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

from pymetis.classes.dataitems.dataitem import DataItem


OCA_KEYWORDS: set[str] = {
    'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
    'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'INS.OPTI11.NAME',
    'DRS.FILTER', 'DRS.IFU',
    'DET.ID', 'DET.DIT', 'DET.NDIT',
    'PRO.CATG',
}


@pytest.mark.dataitem
class DataItemTest:
    """
    Tests for the `DataItem` class hierarchy. These are mostly *class* tests,
    and should not depend on the data provided from SOF or FITS files.
    """
    Item: type[DataItem] = None

    def test_has_title_defined(self):
        assert isinstance(self.Item.title(), str), \
            f"Data item {self.Item.__qualname__} does not define a `title`, or it is not a string"

    def test_has_name_defined(self):
        assert isinstance(self.Item.name(), str), \
            f"Data item {self.Item.__qualname__} does not define a `name`, or it is not a string"

    def test_has_description_defined(self):
        """
        Test that every non-abstract data item defines description():
        by default, it returns the internal `_description` attribute,
        or it can override the getter classmethod.
        """
        assert self.Item.description() is not None, \
            f"Data item {self.Item.__qualname__} does not have a description defined!"

    def test_has_group_defined(self):
        assert isinstance(self.Item.frame_group(), cpl.ui.Frame.FrameGroup), \
            f"Data item {self.Item.__qualname__} does not have a frame group defined!"

    @pytest.mark.metadata
    def test_has_oca_keywords_defined(self):
        assert isinstance(self.Item._oca_keywords, set), \
            f"Data item {self.Item.__qualname__} OCA keywords are not a set"
        #assert len(self.Item._oca_keywords) > 0, \
        #    f"Data item {self.Item.__qualname__} does not define any OCA keywords" # This is actually OK sometimes

    @pytest.mark.metadata
    def test_are_oca_keywords_a_set_of_strings(self):
        for kw in self.Item._oca_keywords:
            assert kw in OCA_KEYWORDS, \
                f"Data item {self.Item.__qualname__} defines an invalid OCA keyword {kw}!"