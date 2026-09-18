"""
The unit vocabulary of the QC parameters.

One spelling per quantity, agreed with the lead (2026-09-18): dimensionless is `None`
(ratios, polynomial degrees), a number of things is "counts" (pixels, lines, exposures),
fluxes are "Jansky" (never "Jy"), lengths on the detector "pixels", wavelengths "Å".
The frozen flat-field module (Gilles' PR) still says "Counts" and is expected to fail
until it is rewritten.
"""
import pytest

import pymetis.instruments.metis.recipes  # noqa: F401  (registers every QC parameter)
from pymetis.engine.qc import QcParameter

UNITS = frozenset({
    None,                   # dimensionless: SNR, Strehl, ellipticity, airmass, chi-squared, polynomial degree
    "counts",               # detector counts, and numbers of pixels / lines / sources / exposures
    "pixels", "pixels^(1 - i)",
    "Jansky", "Jansky / counts", "Jansky / pixel",
    "Å", "Å/pixel^(n + 1)",
    "e / adu",
    "mag", "mas",
})

QC_CLASSES = sorted(
    {klass for klass in [*QcParameter._registry.values(), *QcParameter._templates.values()]
     if not getattr(klass, '_specialized_from', None)},
    key=lambda klass: klass._name_template)


@pytest.mark.parametrize("klass", QC_CLASSES, ids=lambda klass: klass._name_template)
def test_unit_is_in_the_vocabulary(klass, request):
    if klass.__module__.endswith('.qc.flat') and klass._unit == "Counts":
        request.applymarker(pytest.mark.xfail(strict=True, reason="flat-field QC still says 'Counts'; waits for Gilles' PR"))
    assert klass._unit != "undefined", f"{klass.__qualname__} declares no unit"
    assert klass._unit in UNITS, \
        f"{klass.__qualname__} ({klass.name()}) has unit {klass._unit!r}, not in the vocabulary {sorted(map(str, UNITS))}"


def test_a_number_of_things_is_counted_and_a_ratio_is_dimensionless():
    from pymetis.instruments.metis.qc.lingain import LinNumBadpix
    from pymetis.instruments.metis.qc.dark import DarkNBadpix
    from pymetis.instruments.metis.qc.std_process import QcStdStrehl
    from pymetis.instruments.metis.qc.lss import LssSnr
    assert LinNumBadpix._unit == DarkNBadpix._unit == "counts"
    assert QcStdStrehl._unit is None and LssSnr._unit is None


def test_fluxes_are_in_jansky():
    from pymetis.instruments.metis.recipes.ifu.metis_ifu_calibrate import MetisIfuCalibrateImpl
    assert MetisIfuCalibrateImpl.Qc.MinFlux._unit == MetisIfuCalibrateImpl.Qc.MaxFlux._unit == "Jansky"
    assert not any(klass._unit and 'Jy' in klass._unit for klass in QC_CLASSES)
