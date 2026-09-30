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

from pymetis.engine.dataitems.dataitem import DataItem
from pymetis.instruments.metis.dataitems.background import LmStdBackground, NStdBackground, LmSciBackground, \
    NSciBackground, LmStdBackgroundSubtracted, LmSciBackgroundSubtracted, NStdBackgroundSubtracted, \
    NSciBackgroundSubtracted
from pymetis.instruments.metis.dataitems.distortion import DistortionRaw, DistortionTable
from pymetis.instruments.metis.dataitems.adc.adc import LmAdcSlitloss, NAdcSlitloss, LmAdcSlitlossRaw, NAdcSlitlossRaw
from pymetis.instruments.metis.dataitems.common import AtmProfile
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg, MasterDarkGeo, MasterDarkIfu
from pymetis.instruments.metis.dataitems.masterflat import MasterFlat2rg, MasterFlatGeo, MasterFlatIfu
from pymetis.instruments.metis.dataitems.raw import WcuOffRaw
from pymetis.engine.keywords import Keyword


@pytest.mark.dataitem
@pytest.mark.parametrize('item', [LmStdBackground, NStdBackground, LmSciBackground, NSciBackground,
                                  LmStdBackgroundSubtracted, NStdBackgroundSubtracted, LmSciBackgroundSubtracted, NSciBackgroundSubtracted,
                                  WcuOffRaw, AtmProfile, MasterFlatGeo, MasterFlatIfu, MasterFlat2rg,
                                  MasterDark2rg, MasterDarkGeo, MasterDarkIfu, DistortionRaw, DistortionTable,
                                  LmAdcSlitloss, NAdcSlitloss, LmAdcSlitlossRaw, NAdcSlitlossRaw])
class TestDataItem:
    """
    Tests for the `DataItem` class hierarchy. These are mostly *class* tests,
    and should not depend on the data provided from SOF or FITS files.
    """
    Item: type[DataItem] = None

    def test_has_title_defined(self, item):
        assert isinstance(item.title(), str), \
            f"Data item {item.__qualname__} does not define a `title`, or it is not a string"

    @pytest.mark.metadata
    def test_has_name_defined(self, item):
        assert isinstance(item.name(), str), \
            f"Data item {item.__qualname__} does not define a `name`, or it is not a string"

    @pytest.mark.metadata
    def test_has_description_defined(self, item):
        """
        Test that every non-abstract data item defines description():
        by default, it returns the internal `_description` attribute,
        or it can also override the getter classmethod if it has to be computed.
        """
        assert item.description() is not None, \
            f"Data item {item.__qualname__} does not have a description defined!"

    @pytest.mark.metadata
    def test_has_group_defined(self, item):
        assert isinstance(item.frame_group(), cpl.ui.Frame.FrameGroup), \
            f"Data item {item.__qualname__} does not have a frame group defined!"

    @pytest.mark.metadata
    def test_oca_keywords_are_vocabulary_keywords(self, item):
        """ OCA keywords are the keyword objects of the instrument vocabulary, registered under their name. """
        assert isinstance(item.oca_keywords(), frozenset)
        for keyword in item.oca_keywords():
            assert isinstance(keyword, Keyword), f"{item.__qualname__}: OCA keyword {keyword!r} is not a Keyword"
            assert Keyword.registry.get(keyword.name) == keyword, f"{item.__qualname__}: {keyword} is not registered"

    @pytest.mark.metadata
    def test_has_schema_defined(self, item):
        assert isinstance(item._schema, dict), \
            f"Data item {item.__qualname__} does not have a schema defined or it is not a dict!"

    @pytest.mark.metadata
    def test_schema_has_primary_hdu(self, item):
        assert 'PRIMARY' in item._schema.keys(),\
            f"Data item {item.__qualname__} does not have a primary HDU"

    @pytest.mark.metadata
    def test_schema_values_are_classes(self, item):
        for key, klass in item._schema.items():
            assert isinstance(key, str), \
                f"Schema keys must be strings, not {key} ({type(key)})"
            assert klass is not None or key == 'PRIMARY', \
                f"Only the primary header (HDU 0) may be None in schema {item._schema}"
            assert klass in [None, cpl.core.Image, cpl.core.Table], \
                f"The schema type must be CPL Image or Table, not {klass}"



def _registered_items() -> list[type[DataItem]]:
    import pymetis.instruments.metis.recipes  # noqa: F401  (fills the registry)
    return [item for tag, item in sorted(DataItem._registry.items()) if '{' not in tag]


@pytest.mark.dataitem
@pytest.mark.metadata
class TestRegisteredOcaKeywords:
    """
    The OCA keywords of every catalogue item, not just the hand-picked list above:
    an item that forgets `_oca_keywords` inherits the (frozen, empty) default silently.
    """

    @pytest.mark.parametrize('item', _registered_items(), ids=lambda item: item.name())
    def test_oca_keywords_are_a_frozenset_of_vocabulary_keywords(self, item):
        assert isinstance(item.oca_keywords(), frozenset), \
            f"Data item {item.name()} OCA keywords are not a frozenset"
        strangers = {k for k in item.oca_keywords() if not isinstance(k, Keyword) or Keyword.registry.get(k.name) != k}
        assert not strangers, f"Data item {item.name()} defines OCA keywords outside the vocabulary: {strangers}"


@pytest.mark.dataitem
@pytest.mark.metadata
class TestRegisteredFrameGroups:
    @pytest.mark.parametrize('item', _registered_items(), ids=lambda item: item.name())
    def test_a_static_calibration_is_a_calibration(self, item):
        if item.is_static():
            assert item.frame_group() == cpl.ui.Frame.FrameGroup.CALIB, \
                f"Static calibration {item.name()} is in frame group {item.frame_group()}"
