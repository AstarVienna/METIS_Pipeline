"""
Data items and HDUs own their headers: building products from a shared header, or from a
loaded input's header, must not rewrite the caller's object.
"""
import numpy as np
import cpl
from cpl.core import Image as CplImage, PropertyList as CplPropertyList

import pymetis.instruments.metis.dataitems  # noqa: F401  (fills the registry)
from pymetis.engine.dataitems import Hdu
from pymetis.instruments.metis.dataitems.wavecal import IfuWavecal, IfuWavecalTab


class TestDataItemHeader:
    def test_two_products_from_one_header_keep_their_own_category(self):
        header = CplPropertyList()
        a = IfuWavecal(header)
        b = IfuWavecalTab(header)
        assert a.primary_header['ESO PRO CATG'].value == 'IFU_WAVECAL'
        assert b.primary_header['ESO PRO CATG'].value == 'IFU_WAVECAL_TAB'

    def test_the_callers_header_is_untouched(self):
        header = CplPropertyList()
        header.append(cpl.core.Property('ESO PRO CATG', cpl.core.Type.STRING, 'INPUT_ITEM'))
        IfuWavecal(header)
        assert header['ESO PRO CATG'].value == 'INPUT_ITEM'
        assert len(header) == 1


class TestHduHeader:
    def test_extname_is_stamped_on_a_copy(self):
        header = CplPropertyList()
        first = Hdu(header, CplImage(np.ones((2, 2))), name='DET1.SCI')
        second = Hdu(header, CplImage(np.ones((2, 2))), name='DET1.ERR')
        assert first.header['EXTNAME'].value == 'DET1.SCI'
        assert second.header['EXTNAME'].value == 'DET1.ERR'
        assert 'EXTNAME' not in header

    def test_an_unnamed_hdu_is_called_none(self):
        hdu = Hdu(CplPropertyList(), CplImage(np.ones((2, 2))))
        assert hdu.name == 'NONE'
        assert hdu.header['EXTNAME'].value == 'NONE'
