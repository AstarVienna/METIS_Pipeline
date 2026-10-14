"""
Pixels come back from a file in the type the file holds: a DQ bit field as integers with every
bit intact, double data as doubles. Loading everything as 32-bit float, as the loaders once did,
lost DQ bits above bit 23 and truncated double data on every hop between recipes.
"""
import numpy as np
import pytest
import cpl
from cpl.core import Image as CplImage, PropertyList as CplPropertyList
from cpl.ui import Frame

import pymetis.instruments.metis.dataitems  # noqa: F401  (fills the registry)
from pymetis.engine.dataitems.hdu import pixel_type
from pymetis.engine.core.classes.image.base import EnhancedImageBase
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg

DQ_BITS = np.array([[(1 << 30) | 1, (1 << 24) | 1], [(1 << 23) | 1, 0x7FFFFFFF]], dtype=np.int32)
SCI_DOUBLE = np.array([[4.712345678901234, 123456.789], [1e-7, -2.5]], dtype=np.float64)
ERR_FLOAT = np.array([[0.5, 0.25], [0.125, 1.0]], dtype=np.float32)


@pytest.fixture
def master_dark_file(tmp_path):
    path = tmp_path / 'MASTER_DARK_2RG.fits'
    primary = CplPropertyList()
    primary.append(cpl.core.Property('ESO PRO CATG', 'MASTER_DARK_2RG'))
    primary.save(str(path), cpl.core.io.CREATE)
    for name, data in (('DET1.SCI', SCI_DOUBLE), ('DET1.ERR', ERR_FLOAT), ('DET1.DQ', DQ_BITS)):
        header = CplPropertyList()
        header.append(cpl.core.Property('EXTNAME', name))
        CplImage(data).save(str(path), header, cpl.core.io.EXTEND)
    return path


class TestPixelTypeFromHeader:
    @pytest.mark.parametrize('bitpix, scaled, expected', [
        (-64, False, cpl.core.Type.DOUBLE),
        (-32, False, cpl.core.Type.FLOAT),
        (8, False, cpl.core.Type.INT),
        (16, True, cpl.core.Type.INT),       # unsigned 16-bit raw data fits an INT
        (32, False, cpl.core.Type.INT),
        (32, True, cpl.core.Type.DOUBLE),    # unsigned 32-bit would overflow an INT
        (64, False, cpl.core.Type.DOUBLE),   # CPL has no 64-bit integer image
    ])
    def test_bitpix_decides(self, bitpix, scaled, expected):
        header = CplPropertyList()
        header.append(cpl.core.Property('BITPIX', bitpix))
        if scaled:
            header.append(cpl.core.Property('BZERO', 32768))
        assert pixel_type(header) == expected

    def test_without_bitpix_it_is_float(self):
        assert pixel_type(CplPropertyList()) == cpl.core.Type.FLOAT


class TestDataItemLoadData:
    def test_dq_bits_survive(self, master_dark_file):
        item = MasterDark2rg.load(Frame(str(master_dark_file), tag='MASTER_DARK_2RG'))
        dq = item.load_data('DET1.DQ')
        assert dq.type == cpl.core.Type.INT
        assert np.array_equal(np.asarray(dq), DQ_BITS)

    def test_double_data_is_exact(self, master_dark_file):
        item = MasterDark2rg.load(Frame(str(master_dark_file), tag='MASTER_DARK_2RG'))
        sci = item.load_data('DET1.SCI')
        assert sci.type == cpl.core.Type.DOUBLE
        assert np.array_equal(np.asarray(sci), SCI_DOUBLE)

    def test_float_data_stays_float(self, master_dark_file):
        item = MasterDark2rg.load(Frame(str(master_dark_file), tag='MASTER_DARK_2RG'))
        err = item.load_data('DET1.ERR')
        assert err.type == cpl.core.Type.FLOAT
        assert np.array_equal(np.asarray(err), ERR_FLOAT)


class TestEnhancedImageLoad:
    def test_layers_keep_their_types(self, master_dark_file):
        image = EnhancedImageBase.load(str(master_dark_file), 'DET1')
        assert np.array_equal(image.image.image.as_array(), SCI_DOUBLE)      # exact: no float32 truncation
        assert np.array_equal(image.dq.data.as_array(), DQ_BITS)


class TestSavingALoadedItem:
    def test_extensions_are_read_back_from_the_file(self, master_dark_file, tmp_path):
        """ A loaded item holds no pixels; saving it (a re-tagged copy) reads them from its file (Review III, G3). """
        item = MasterDark2rg.load(Frame(str(master_dark_file), tag='MASTER_DARK_2RG'))
        copy = tmp_path / 'copy.fits'
        CplPropertyList().save(str(copy), cpl.core.io.CREATE)
        item.save_extensions(str(copy))
        again = MasterDark2rg.load(Frame(str(copy), tag='MASTER_DARK_2RG'))
        assert np.array_equal(np.asarray(again.load_data('DET1.DQ')), DQ_BITS)
        assert np.array_equal(np.asarray(again.load_data('DET1.SCI')), SCI_DOUBLE)
