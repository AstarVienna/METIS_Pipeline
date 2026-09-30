"""
The Keyword object: one canonical name, every spelling derived, header access typed,
aliases resolving to instrument keywords, the registry refusing a second declaration.
"""
import cpl
import pytest

from pymetis.engine.keywords import Keyword, Alias
from pymetis.engine.keywords import eso


@pytest.fixture
def header() -> cpl.core.PropertyList:
    h = cpl.core.PropertyList()
    h.append(cpl.core.Property('ESO DET DIT', cpl.core.Type.DOUBLE, 0.25))
    h.append(cpl.core.Property('ESO DET NDIT', cpl.core.Type.INT, 4))
    h.append(cpl.core.Property('MJD-OBS', cpl.core.Type.DOUBLE, 60000.5))
    h.append(cpl.core.Property('ESO INS OPTI10 NAME', cpl.core.Type.STRING, 'Lp'))
    h.append(cpl.core.Property('ESO SEQ WCU LASER2 WLEN', cpl.core.Type.DOUBLE, 4.71))
    return h


class TestSpellings:
    def test_dotted_header_hierarch_edps(self):
        assert eso.DET_DIT.dotted == 'DET.DIT'
        assert eso.DET_DIT.header == 'ESO DET DIT'
        assert eso.DET_DIT.hierarch == 'HIERARCH ESO DET DIT'
        assert eso.DET_DIT.edps == 'det.dit'

    def test_a_bare_name_stays_bare(self):
        assert eso.MJD_OBS.header == 'MJD-OBS'
        assert eso.MJD_OBS.hierarch == 'MJD-OBS'
        assert eso.MJD_OBS.edps == 'mjd-obs'
        assert eso.MJD_OBS.group == 'FITS'

    def test_group_is_the_first_component(self):
        assert eso.DET_DIT.group == 'DET' and eso.PRO_CATG.group == 'PRO'

    def test_lookups_invert_the_spellings(self):
        assert Keyword.find('DET.DIT') is eso.DET_DIT
        assert Keyword.from_edps('det.dit') is eso.DET_DIT
        assert Keyword.from_header('ESO DET DIT') is eso.DET_DIT
        assert Keyword.from_header('HIERARCH ESO DET DIT') is eso.DET_DIT
        assert Keyword.from_header('MJD-OBS') is eso.MJD_OBS
        assert Keyword.find('NO.SUCH.KEYWORD') is None

    def test_printf_defaults_from_the_type(self):
        assert eso.DET_NDIT.printf == '%i' and eso.DET_DIT.printf == '%.3f' and eso.MJD_OBS.printf == '%.8f'

    def test_canonical_form_is_enforced(self):
        with pytest.raises(ValueError):
            Keyword('ESO DET DIT')
        with pytest.raises(ValueError):
            Keyword('det.dit')


class TestIdentity:
    def test_compared_and_hashed_by_name_only(self):
        a = Keyword('TEST.KW.ONE', float, description='a')
        b = Keyword('TEST.KW.ONE', float, description='a')     # identical redefinition: allowed (module reloads)
        assert a == b and hash(a) == hash(b) and len({a, b}) == 1

    def test_a_different_second_declaration_is_refused(self):
        Keyword('TEST.KW.TWO', float)
        with pytest.raises(TypeError):
            Keyword('TEST.KW.TWO', int)

    def test_sorted_by_name_and_str(self):
        assert sorted([eso.PRO_CATG, eso.DET_DIT]) == [eso.DET_DIT, eso.PRO_CATG]
        assert str(eso.DET_DIT) == 'DET.DIT'


class TestIndexed:
    LASER = Keyword('TEST.LASER{n}.WLEN', float, unit='um', index=range(1, 5))

    def test_resolves_and_points_back(self):
        two = self.LASER[2]
        assert two.name == 'TEST.LASER2.WLEN' and two.header == 'ESO TEST LASER2 WLEN'
        assert two.template is self.LASER and two.unit == 'um'
        assert two == self.LASER[2] and two.name not in Keyword.registry

    def test_index_range_and_non_templates(self):
        with pytest.raises(IndexError):
            self.LASER[7]
        with pytest.raises(TypeError):
            eso.DET_DIT[1]
        with pytest.raises(ValueError):
            Keyword('TEST.NOT.INDEXED', index=range(1, 3))

    def test_find_goes_through_the_template(self):
        found = Keyword.find('TEST.LASER3.WLEN')
        assert found is not None and found.template is self.LASER and found.name == 'TEST.LASER3.WLEN'
        assert Keyword.from_header('ESO TEST LASER3 WLEN') == found

    def test_shown_and_label(self):
        assert self.LASER.shown == 'TEST.LASERn.WLEN' and self.LASER.label == 'test.lasern.wlen'


class TestHeaderAccess:
    def test_get_coerces_to_the_declared_type(self, header):
        assert eso.DET_DIT.get(header) == 0.25 and isinstance(eso.DET_NDIT.get(header), int)
        assert eso.MJD_OBS.present(header) and not eso.EXPTIME.present(header)

    def test_missing_card(self, header):
        with pytest.raises(KeyError, match='ESO EXPTIME|EXPTIME'):
            eso.EXPTIME.get(header)
        assert eso.EXPTIME.get(header, default=None) is None

    def test_set_appends_or_replaces(self, header):
        eso.PRO_CATG.set(header, 'MASTER_DARK_2RG')
        assert header['ESO PRO CATG'].value == 'MASTER_DARK_2RG'
        eso.PRO_CATG.set(header, 'MASTER_DARK_GEO')
        assert header['ESO PRO CATG'].value == 'MASTER_DARK_GEO'
        assert sum(1 for p in header if p.name == 'ESO PRO CATG') == 1

    def test_indexed_get(self, header):
        # the instrument vocabulary may already have declared it in this session; a second identical declaration is fine
        kw = Keyword.find('SEQ.WCU.LASER{n}.WLEN') or Keyword('SEQ.WCU.LASER{n}.WLEN', float, index=range(1, 5))
        assert kw[2].get(header) == 4.71


class TestAlias:
    OPTI_A = Keyword('TEST.OPTI1.NAME')
    OPTI_B = Keyword('TEST.OPTI2.NAME')
    ANY = Alias('DRS.TESTANY', resolves_to=(OPTI_A, OPTI_B))
    ALL = Alias('DRS.TESTALL', resolves_to=(OPTI_A, OPTI_B), combine='all')

    def test_any_takes_whichever_is_present(self):
        h = cpl.core.PropertyList()
        h.append(cpl.core.Property('ESO TEST OPTI2 NAME', cpl.core.Type.STRING, 'N2'))
        assert self.ANY.get(h) == 'N2'
        assert self.ANY.edps_alternatives == ('test.opti1.name', 'test.opti2.name')

    def test_all_gives_every_present_value(self):
        h = cpl.core.PropertyList()
        h.append(cpl.core.Property('ESO TEST OPTI1 NAME', cpl.core.Type.STRING, 'A'))
        h.append(cpl.core.Property('ESO TEST OPTI2 NAME', cpl.core.Type.STRING, 'B'))
        assert self.ALL.get(h) == ('A', 'B')

    def test_a_physical_alias_card_wins(self):
        h = cpl.core.PropertyList()
        h.append(cpl.core.Property('ESO DRS TESTANY', cpl.core.Type.STRING, 'simulated'))
        h.append(cpl.core.Property('ESO TEST OPTI1 NAME', cpl.core.Type.STRING, 'A'))
        assert self.ANY.get(h) == 'simulated'

    def test_nothing_present(self):
        h = cpl.core.PropertyList()
        with pytest.raises(KeyError):
            self.ANY.get(h)
        assert self.ANY.get(h, default='open') == 'open'

    def test_aliases_are_drs_and_resolve_to_concrete_keywords(self):
        with pytest.raises(ValueError):
            Alias('INS.NOTALIAS', resolves_to=(self.OPTI_A,))
        with pytest.raises(ValueError):
            Alias('DRS.EMPTY')
        with pytest.raises(ValueError):
            Alias('DRS.NESTED', resolves_to=(self.ANY,))
