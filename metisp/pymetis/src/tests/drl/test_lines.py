"""
Unit tests for emission line detection and measurement.

Each test exercises one property: sub-pixel position accuracy, width and height
accuracy, independence of the reported width from the pre-smoothing, rejection of
line-free spectra, the effect of each filter, and the Gaussian fit's seeding.
"""
import numpy as np
import pytest

from pymetis.drl.lines import (FWHM_PER_SIGMA, detect_lines, fit_gaussian,
                               prepare_spectrum)

NPIX = 2048
BACKGROUND = 200.0
NOISE = 20.0


def spectrum(lines=((300.4, 3.0, 5000.0),), noise=NOISE, seed=3, npix=NPIX):
    """
    A synthetic spectrum with Gaussian emission lines on a noisy background.

    `lines` is a sequence of `(centre, sigma, amplitude)`.
    """
    rng = np.random.default_rng(seed)
    x = np.arange(float(npix))
    values = np.full(npix, BACKGROUND)

    for centre, sigma, amplitude in lines:
        values += amplitude * np.exp(-0.5 * ((x - centre) / sigma) ** 2)

    return values + rng.normal(0, noise, npix)


class TestDetectLines:
    def test_finds_every_line(self):
        lines = ((300.4, 3.0, 5000.0), (900.7, 3.0, 3000.0), (1500.2, 3.0, 8000.0))

        detected = detect_lines(spectrum(lines))

        assert len(detected) == len(lines)

    def test_positions_are_subpixel_accurate(self):
        lines = ((300.4, 3.0, 5000.0), (900.7, 3.0, 3000.0), (1500.2, 3.0, 8000.0))

        detected = detect_lines(spectrum(lines))

        for line, (centre, _, _) in zip(detected, lines):
            assert abs(line.position - centre) < 0.1

    def test_returns_lines_ordered_by_position(self):
        lines = ((1500.2, 3.0, 8000.0), (300.4, 3.0, 5000.0), (900.7, 3.0, 3000.0))

        detected = detect_lines(spectrum(lines))

        assert [line.position for line in detected] == sorted(
            line.position for line in detected)

    def test_heights_match_the_line_amplitudes(self):
        lines = ((300.4, 3.0, 5000.0), (900.7, 3.0, 3000.0))

        detected = detect_lines(spectrum(lines))

        for line, (_, _, amplitude) in zip(detected, lines):
            assert line.height == pytest.approx(amplitude, rel=0.05)

    def test_widths_match_the_line_widths(self):
        for sigma in (2.0, 3.0, 5.0):
            detected = detect_lines(spectrum(((1000.0, sigma, 4000.0),)))

            assert len(detected) == 1
            assert detected[0].fwhm == pytest.approx(FWHM_PER_SIGMA * sigma, rel=0.05)

    def test_width_is_independent_of_smoothing(self):
        """
        The reported width must describe the line, not the detection kernel.

        `QC IFU WAVECAL LINE WIDTH` is tied to the spectral resolution requirement, so a
        width inflated by the pipeline's own smoothing would misreport the instrument.
        """
        data = spectrum(((1000.0, 3.0, 4000.0),))
        expected = FWHM_PER_SIGMA * 3.0

        for smoothing in (0.0, 1.0, 2.0):
            detected = detect_lines(data, smoothing=smoothing)

            assert len(detected) == 1
            assert detected[0].fwhm == pytest.approx(expected, rel=0.05)

    def test_records_the_offset_it_was_given(self):
        detected = detect_lines(spectrum(), offset=-12.5)

        assert detected
        assert all(line.offset == -12.5 for line in detected)

    def test_wavelength_starts_unassigned(self):
        detected = detect_lines(spectrum())

        assert detected
        assert all(line.wavelength is None for line in detected)

    def test_line_after_a_flat_run_is_still_detected(self):
        """
        A flat run -- e.g. a masked-then-zero-filled region before the trace's valid
        columns start -- has no local minimum of its own, since a strict extremum needs
        a neighbour that differs. A line sitting right after one must not be discarded
        just because it therefore has no minimum bracketing it on that side.
        """
        rng = np.random.default_rng(3)
        values = np.full(NPIX, BACKGROUND)
        values[600:] += rng.normal(0, NOISE, NPIX - 600)
        values += 5000.0 * np.exp(-0.5 * ((np.arange(NPIX) - 620.0) / 3.0) ** 2)

        detected = detect_lines(values)

        assert len(detected) == 1
        assert detected[0].position == pytest.approx(620.0, abs=0.1)


class TestDetectLinesRejection:
    """Spectra with no line must yield nothing, so that NLINES stays trustworthy."""

    def test_pure_noise_yields_no_lines(self):
        for seed in range(20):
            assert detect_lines(spectrum(lines=(), seed=seed)) == []

    def test_flat_spectrum_yields_no_lines(self):
        assert detect_lines(np.zeros(NPIX)) == []
        assert detect_lines(np.full(NPIX, BACKGROUND)) == []

    def test_short_spectrum_yields_no_lines(self):
        assert detect_lines(np.zeros(5)) == []

    def test_line_below_the_noise_floor_is_rejected(self):
        """A 3-sigma bump is not a detection at the default min_snr of 5."""
        assert detect_lines(spectrum(((1000.0, 3.0, 3 * NOISE),))) == []

    def test_min_snr_controls_the_noise_floor(self):
        data = spectrum(((1000.0, 3.0, 20 * NOISE),))

        assert len(detect_lines(data, min_snr=3.0)) == 1
        assert detect_lines(data, min_snr=50.0) == []

    def test_edge_margin_excludes_lines_near_the_ends(self):
        data = spectrum(((5.0, 3.0, 8000.0), (1000.0, 3.0, 8000.0)))

        detected = detect_lines(data, edge_margin=50)

        assert len(detected) == 1
        assert detected[0].position == pytest.approx(1000.0, abs=0.5)

    def test_width_limits_exclude_out_of_range_lines(self):
        data = spectrum(((1000.0, 5.0, 8000.0),))

        assert detect_lines(data, fwhm_max=5.0) == []
        assert detect_lines(data, fwhm_min=20.0) == []
        assert len(detect_lines(data, fwhm_min=1.0, fwhm_max=30.0)) == 1

    def test_cutoff_excludes_faint_lines_relative_to_the_brightest(self):
        data = spectrum(((300.0, 3.0, 10000.0), (1000.0, 3.0, 500.0)))

        assert len(detect_lines(data, cutoff=0.0)) == 2
        assert len(detect_lines(data, cutoff=0.5)) == 1


class TestPrepareSpectrum:
    def test_subtracts_the_background_and_clips(self):
        prepared, _ = prepare_spectrum(np.full(100, 50.0))

        np.testing.assert_allclose(prepared, 0.0)

    def test_estimates_the_noise(self):
        rng = np.random.default_rng(0)

        _, noise = prepare_spectrum(rng.normal(200, 20, 20000))

        assert noise == pytest.approx(20.0, rel=0.05)

    def test_noise_estimate_survives_bright_lines(self):
        """The lines occupy few pixels, so a robust estimator must ignore them."""
        _, noise = prepare_spectrum(spectrum(((1000.0, 3.0, 100000.0),)))

        assert noise == pytest.approx(NOISE, rel=0.15)

    def test_handles_nan_and_masked_input(self):
        values = np.full(100, 10.0)
        values[5] = np.nan

        prepared, _ = prepare_spectrum(values)
        assert np.all(np.isfinite(prepared))

        masked = np.ma.masked_array(np.full(100, 10.0), mask=False)
        masked.mask[5] = True

        prepared, _ = prepare_spectrum(masked)
        assert np.all(np.isfinite(prepared))


class TestFitGaussian:
    def test_recovers_gaussian_parameters(self):
        x = np.arange(80.0)
        y = 500.0 * np.exp(-0.5 * ((x - 40.3) / 4.0) ** 2) + 20.0

        height, centre, variance, background = fit_gaussian(x, y)

        assert height == pytest.approx(500.0, rel=1e-3)
        assert centre == pytest.approx(40.3, abs=1e-3)
        assert np.sqrt(variance) == pytest.approx(4.0, rel=1e-3)
        assert background == pytest.approx(20.0, abs=1.0)

    def test_seeding_rescues_an_off_centre_peak(self):
        """
        Without a seed, a peak far from the segment centre is missed.

        Upstream PyReduce seeds from the brightest point in the middle half of the
        segment. When the line sits near one end, that heuristic converges to a spurious
        narrow spike instead, which is why the caller passes the peak position.
        """
        x = np.arange(140.0)
        y = 500.0 * np.exp(-0.5 * ((x - 15.0) / 3.0) ** 2) + 20.0

        _, seeded, seeded_variance, _ = fit_gaussian(x, y, centre_guess=15.0)
        assert seeded == pytest.approx(15.0, abs=0.01)
        assert np.sqrt(seeded_variance) == pytest.approx(3.0, rel=0.01)

        # Unseeded, the fit either fails to converge or lands on the wrong feature
        try:
            _, unseeded, _, _ = fit_gaussian(x, y)
        except RuntimeError:
            return
        assert abs(unseeded - 15.0) > 1.0

    def test_rejects_too_few_points(self):
        with pytest.raises(ValueError, match='at least 4'):
            fit_gaussian(np.arange(3.0), np.ones(3))
