"""
An item's kind (CPL frame type) and its schema must agree, mixins carry tags only, and
product file names stay within the DICD limit.
"""
import cpl
import pytest
from cpl.core import Image, ImageList, Table

import pymetis.instruments.metis.recipes  # noqa: F401  (fills the registry)
from pymetis.engine.dataitems import DataItem, ImageDataItem, TableDataItem, detectors
from pymetis.engine.core.parametrizable import Parametrizable
import pymetis.instruments.metis.mixins as mixins

ITEMS = [item for tag, item in sorted(DataItem._registry.items())]


class TestKindConsistency:
    def test_an_image_item_holding_a_table_is_refused_at_import(self):
        with pytest.raises(TypeError, match="schema holds \\['Table'\\]"):
            class Probe(ImageDataItem):   # noqa: F841
                _name_template = 'PROBE_IMAGE_WITH_TABLE'
                _frame_group = cpl.ui.Frame.FrameGroup.CALIB
                _frame_level = cpl.ui.Frame.FrameLevel.FINAL
                _schema = {'PRIMARY': None, 'TABLE': Table}

    def test_a_table_item_holding_images_is_refused_at_import(self):
        with pytest.raises(TypeError, match="ImageDataItem"):
            class Probe(TableDataItem):   # noqa: F841
                _name_template = 'PROBE_TABLE_WITH_IMAGES'
                _frame_group = cpl.ui.Frame.FrameGroup.CALIB
                _frame_level = cpl.ui.Frame.FrameLevel.FINAL
                _schema = detectors(Image, 2)

    @pytest.mark.parametrize('item', ITEMS, ids=lambda item: item.name())
    def test_every_registered_item_is_consistent(self, item):
        kinds = {klass for klass in item.schema().values() if klass is not None}
        if item.frame_type() == cpl.ui.Frame.FrameType.IMAGE:
            assert Table not in kinds
        if item.frame_type() == cpl.ui.Frame.FrameType.TABLE:
            assert not kinds & {Image, ImageList}


class TestDeclarations:
    def test_no_mixin_carries_a_schema(self):
        for name in dir(mixins):
            klass = getattr(mixins, name)
            if isinstance(klass, type) and issubclass(klass, Parametrizable) and name.endswith('Mixin'):
                assert '_schema' not in klass.__dict__, f"{name} carries a schema; mixins carry tags only"

    def test_the_detectors_factory(self):
        assert detectors(Image, 2) == {'PRIMARY': None, 'DET1.DATA': Image, 'DET2.DATA': Image}
        assert detectors(Table, 1, 'SCI') == {'PRIMARY': None, 'DET1.SCI': Table}

    @pytest.mark.parametrize('item', ITEMS, ids=lambda item: item.name())
    def test_product_file_names_fit_the_dicd_limit(self, item):
        """ <TAG>_<21-character timestamp>.fits must not exceed 56 characters. """
        assert len(item.name()) + 1 + 21 + 5 <= 56, f"{item.name()} makes a file name longer than 56 characters"
