"""
QC parameter values are coerced into their declared type, and a value that could not be
determined (None) is reported but never written.
"""
from types import SimpleNamespace

import numpy as np
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401
from pymetis.engine.recipes import RecipeImpl
from pymetis.instruments.metis.qc.dark import DarkMean, DarkNBadpix


class TestCoercion:
    def test_numpy_scalars_are_accepted(self):
        assert DarkMean(np.float32(1.5)).value == 1.5 and type(DarkMean(np.float32(1.5)).value) is float
        assert DarkNBadpix(np.int64(3)).value == 3 and type(DarkNBadpix(np.int64(3)).value) is int

    def test_an_integer_is_a_fine_float(self):
        assert DarkMean(2).value == 2.0 and type(DarkMean(2).value) is float

    def test_a_bool_is_not_a_count(self):
        with pytest.raises(ValueError):
            DarkNBadpix(True)

    def test_a_string_is_not_a_number(self):
        with pytest.raises(ValueError):
            DarkMean("1.5")


class TestNotAvailable:
    def test_none_means_not_available(self):
        qc = DarkMean(None)
        assert not qc.available
        assert qc.as_property() is None

    def test_collect_skips_unavailable_values(self):
        collected = RecipeImpl.collect_qc_parameters(SimpleNamespace(), DarkMean(1.0), DarkNBadpix(None), DarkNBadpix(4))
        assert [prop.name for prop in collected] == ['QC DARK MEAN', 'QC DARK NBADPIX']


class TestIndexPlaceholders:
    def test_a_missing_value_needs_no_resolved_index(self):
        """ `self.Qc.LCoeff(None)` is the skeleton's placeholder for LCOEFF{order}; nothing is
        written, so the index may stay open. A real value still needs `.specialized(order=n)`. """
        from pymetis.instruments.metis.qc.trace import QcLssTraceLCoeff
        klass = QcLssTraceLCoeff.specialized(band='LM')
        assert klass(None).available is False
        with pytest.raises(TypeError, match="still has placeholders"):
            klass(1.0)
        assert klass.specialized(order=2)(1.0).name() == 'QC LM LSS TRACE LCOEFF2'
