"""
Tests for reconstructing a `DataItem` from a FITS file via `DataItem.load`.
"""
import numpy as np

import cpl
import pytest
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


class TestExtensionsWithoutExtname:
    """ The simulated calibration tables carry one BINTABLE without EXTNAME; extensions
    without a name are matched to the schema by position, never called after XTENSION. """

    @staticmethod
    def _write(filename, catg, *hdus):
        from astropy.io import fits
        primary = fits.PrimaryHDU()
        primary.header['HIERARCH ESO PRO CATG'] = catg
        fits.HDUList([primary, *hdus]).writeto(filename, overwrite=True)

    def test_a_single_nameless_table_lands_on_the_only_data_extension(self, tmp_path):
        import numpy as np
        from astropy.io import fits
        from pymetis.instruments.metis.dataitems.common import LsfKernel
        filename = str(tmp_path / 'LSF_KERNEL.fits')
        self._write(filename, 'LSF_KERNEL', fits.BinTableHDU.from_columns(
            [fits.Column(name='wavelength', format='D', array=np.arange(3.0))]))
        item = LsfKernel.load(cpl.ui.Frame(filename, tag='LSF_KERNEL'))
        assert list(item.hdus) == ['PRIMARY', 'TABLE']
        assert isinstance(item.load_data('TABLE'), cpl.core.Table)

    def test_several_nameless_extensions_map_in_schema_order(self, tmp_path):
        import numpy as np
        from astropy.io import fits
        from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg
        filename = str(tmp_path / 'MASTER_DARK_2RG.fits')
        self._write(filename, 'MASTER_DARK_2RG', *[fits.ImageHDU(np.full((2, 2), v)) for v in (1.0, 2.0, 3.0)])
        item = MasterDark2rg.load(cpl.ui.Frame(filename, tag='MASTER_DARK_2RG'))
        assert list(item.hdus) == ['PRIMARY', 'DET1.SCI', 'DET1.ERR', 'DET1.DQ']

    def test_more_nameless_extensions_than_the_schema_has_is_a_format_error(self, tmp_path):
        import numpy as np
        from astropy.io import fits
        from pymetis.instruments.metis.dataitems.common import LsfKernel
        filename = str(tmp_path / 'LSF_KERNEL.fits')
        self._write(filename, 'LSF_KERNEL', fits.ImageHDU(np.zeros((2, 2))), fits.ImageHDU(np.zeros((2, 2))))
        with pytest.raises(cpl.core.BadFileFormatError, match='has no EXTNAME'):
            LsfKernel.load(cpl.ui.Frame(filename, tag='LSF_KERNEL'))

    def test_a_wrong_extension_kind_is_a_format_error_not_an_assertion(self, tmp_path):
        import numpy as np
        from astropy.io import fits
        from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark2rg
        filename = str(tmp_path / 'MASTER_DARK_2RG.fits')
        table = fits.BinTableHDU.from_columns([fits.Column(name='x', format='D', array=np.arange(2.0))], name='DET1.SCI')
        self._write(filename, 'MASTER_DARK_2RG', table)
        with pytest.raises(cpl.core.BadFileFormatError, match="DET1.SCI"):
            MasterDark2rg.load(cpl.ui.Frame(filename, tag='MASTER_DARK_2RG'))
