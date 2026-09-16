"""
Tests for reconstructing a `DataItem` from a FITS file via `DataItem.load`.
"""
import numpy as np

import cpl
from cpl.core import Image as CplImage, PropertyList as CplPropertyList

from pymetis.engine.dataitems import Hdu
from pymetis.instruments.metis.dataitems.common import PersistenceMap


def write_persistence_map(filename: str, pro_catg: str | None = None) -> None:
    """ A PERSISTENCE_MAP file: empty primary HDU plus the image extension. """
    primary = CplPropertyList()
    if pro_catg is not None:
        primary.append(cpl.core.Property("ESO PRO CATG", cpl.core.Type.STRING, pro_catg))
    primary.save(filename, cpl.core.io.CREATE)
    Hdu(CplPropertyList(), CplImage(np.zeros((4, 6))), name='PERSISTENCE_MAP').save(filename)


class TestDataItemLoad:
    def test_loads_every_hdu_by_extname(self, tmp_path):
        filename = str(tmp_path / 'persistence.fits')
        write_persistence_map(filename)

        item = PersistenceMap.load(cpl.ui.Frame(filename, tag='PERSISTENCE_MAP'))

        assert set(item.hdus) == {'PRIMARY', 'PERSISTENCE_MAP'}
        assert item.hdus['PERSISTENCE_MAP'].klass is CplImage

    def test_pro_catg_is_set_exactly_once(self, tmp_path):
        """ Loading a file that already carries ESO PRO CATG must replace the
        card, not append a second one that would be saved along. """
        filename = str(tmp_path / 'persistence_catg.fits')
        write_persistence_map(filename, pro_catg='STALE_CATG')

        item = PersistenceMap.load(cpl.ui.Frame(filename, tag='PERSISTENCE_MAP'))

        cards = [prop for prop in item.primary_header if prop.name == 'ESO PRO CATG']
        assert len(cards) == 1
        assert cards[0].value == 'PERSISTENCE_MAP'
