"""
This file is part of an A* Pipeline.
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

Detection and sub-pixel measurement of emission lines in a 1D spectrum.

Adapted from PyReduce (`pyreduce.wavelength_calibration.WavelengthCalibrationInitialize`
and `pyreduce.util.gaussfit3`), whose wavelength calibration the METIS DRLD prescribes
for the IFU. The DRLD asks for line locations measured as a "centroid by Gaussian fit",
which is what `fit_gaussian` provides.

Original authors: Nikolai Piskunov, Thomas Marquart, Ansgar Wehrhahn (GPLv3).

Differences from upstream PyReduce
----------------------------------
- Detection is decoupled from identification. Upstream immediately matches peaks
  against a reference atlas; here the measurement is returned on its own, because the
  METIS IFU lasers have known wavelengths and need no atlas cross-matching, and because
  the measured widths and heights are quality control parameters in their own right.
- Line heights are reported in counts above the local background rather than on a
  peak-normalised scale, so that `QC IFU WAVECAL PEAK CNTS` is a physical quantity.
- Peak finding is vectorised rather than looped over pixels.
"""

import warnings
from dataclasses import dataclass

import numpy as np
from scipy.ndimage import gaussian_filter1d
from scipy.optimize import curve_fit

# Full width at half maximum of a Gaussian, in units of its standard deviation
FWHM_PER_SIGMA = 2.0 * np.sqrt(2.0 * np.log(2.0))


@dataclass
class Line:
    """
    A single emission line measured in one extracted spectrum.

    Attributes
    ----------
    position : float
        Sub-pixel centre along the dispersion direction, in pixels.
    fwhm : float
        Full width at half maximum, in pixels, with any pre-smoothing applied during
        detection removed in quadrature. Feeds `QC IFU WAVECAL LINE WIDTH`.
    height : float
        Peak amplitude in counts above the local background, corrected for any
        pre-smoothing applied during detection. Feeds `QC IFU WAVECAL PEAK CNTS`.
    offset : float
        Cross-dispersion position at which this line was measured, in pixels relative
        to the mid-line of the slice. Zero for a spectrum with no offset context.
    wavelength : float | None
        Wavelength assigned to this line, once identified.
    """

    position: float
    fwhm: float
    height: float
    offset: float = 0.0
    wavelength: float | None = None


def gaussian(x: np.ndarray, height: float, centre: float,
             variance: float, background: float) -> np.ndarray:
    """
    A Gaussian on a constant background.

    Note that the third parameter is the *variance*, not the standard deviation. This
    follows PyReduce's `gaussval2`, and keeps the fit insensitive to the sign of the
    parameter, which helps the optimiser.
    """
    return height * np.exp(-((x - centre) ** 2) / (2 * variance)) + background


def fit_gaussian(x: np.ndarray,
                 y: np.ndarray,
                 centre_guess: float | None = None) -> tuple[float, float, float, float]:
    """
    Fit a Gaussian on a constant background to a short segment of a spectrum.

    Parameters
    ----------
    x, y : np.ndarray
        Segment to fit. Masked entries are dropped.
    centre_guess : float, optional
        Initial guess for the line centre. Strongly preferred when the caller already
        knows which peak is being fitted: upstream PyReduce instead seeds from the
        brightest point in the middle half of the segment, which silently converges to
        a spurious narrow spike whenever the peak does not lie near the segment centre.

    Returns
    -------
    tuple[float, float, float, float]
        Height, centre, variance and background.

    Raises
    ------
    RuntimeError, ValueError
        If the fit does not converge, or too few points remain to fit four parameters.
    """
    mask = np.ma.getmaskarray(x) | np.ma.getmaskarray(y)
    x, y = np.asarray(x)[~mask], np.asarray(y)[~mask]

    if x.size < 4:
        raise ValueError(f"Need at least 4 points to fit a Gaussian, got {x.size}")

    if centre_guess is None:
        lo, hi = len(y) // 4, len(y) * 3 // 4
        peak = int(np.argmax(y[lo:hi])) + lo if hi > lo else int(np.argmax(y))
    else:
        peak = int(np.argmin(np.abs(x - centre_guess)))

    guess = [y[peak] - np.min(y), x[peak], 1.0, float(np.min(y))]

    with warnings.catch_warnings():
        warnings.simplefilter('ignore')
        parameters, _ = curve_fit(gaussian, x, y, p0=guess)

    return tuple(parameters)


def _local_extrema(spectrum: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Return the indices of strict local maxima and minima of a 1D array."""
    interior = spectrum[1:-1]
    maxima = np.flatnonzero((interior > spectrum[:-2]) & (interior > spectrum[2:])) + 1
    minima = np.flatnonzero((interior < spectrum[:-2]) & (interior < spectrum[2:])) + 1
    return maxima, minima


def prepare_spectrum(spectrum: np.ndarray,
                     smoothing: float = 0.0) -> tuple[np.ndarray, float]:
    """
    Subtract the background from a spectrum, optionally smooth it, and estimate its noise.

    The median is used as the background, which is appropriate for the sparse emission
    lines of the WCU lasers: the vast majority of pixels see no line. Negative
    excursions are then clipped, as upstream does, so that noise troughs cannot be
    mistaken for the minima that bracket a line.

    Returns
    -------
    tuple[np.ndarray, float]
        The background-subtracted, clipped spectrum, and a robust estimate of its noise
        standard deviation. The noise is measured *before* clipping, since clipping
        removes half the distribution and would halve the estimate.
    """
    if np.ma.isMaskedArray(spectrum):
        spectrum = np.ma.filled(spectrum.astype(float), 0.0)
    else:
        spectrum = np.nan_to_num(np.asarray(spectrum, dtype=float), nan=0.0, copy=True)

    spectrum = spectrum - np.median(spectrum)

    if smoothing != 0:
        spectrum = gaussian_filter1d(spectrum, smoothing)

    # Median absolute deviation about zero, scaled to a Gaussian sigma. Robust against
    # the lines themselves, which occupy only a small fraction of the pixels.
    noise = 1.4826 * float(np.median(np.abs(spectrum)))

    return np.clip(spectrum, 0.0, None), noise


def detect_lines(spectrum: np.ndarray,
                 *,
                 offset: float = 0.0,
                 smoothing: float = 1.0,
                 cutoff: float = 0.1,
                 min_snr: float = 5.0,
                 edge_margin: int = 10,
                 min_separation: int = 3,
                 fwhm_min: float = 1.5,
                 fwhm_max: float = 20.0) -> list[Line]:
    """
    Find emission lines in a 1D spectrum and measure each by a Gaussian fit.

    A peak is kept only if it is a strict local maximum, sits at least `edge_margin`
    pixels from either end, is separated from the nearest local minimum on each side by
    at least `min_separation` pixels, rises above both the `cutoff` threshold and
    `min_snr` times the noise, and yields a Gaussian fit whose width falls inside
    `[fwhm_min, fwhm_max]`. The separation and width tests reject noise spikes, which
    are unresolved; the signal to noise test rejects a spectrum that contains no line
    at all.

    The noise floor is an addition to upstream PyReduce, which thresholds only at a
    fraction of the brightest peak. That test degenerates on a line-free spectrum,
    where the brightest peak is itself noise, and yields a crop of spurious lines.

    Parameters
    ----------
    spectrum : np.ndarray
        Extracted 1D spectrum.
    offset : float
        Cross-dispersion position this spectrum was extracted at, recorded on each
        returned line so that a two-dimensional solution can be fitted later.
    smoothing : float
        Standard deviation of a Gaussian pre-smoothing, in pixels. Without it, noise
        rides on the flanks of a faint line and creates spurious local minima right
        beside the peak, which the separation test then rejects. The default of 1.0
        detects lines down to a signal to noise ratio of about 10 while introducing no
        false positives; raise it to reach fainter lines.
    cutoff : float
        Detection threshold. Below 1 it is a fraction of the brightest peak; at or
        above 1 it is a percentile of the non-zero pixels. Zero disables the test.
    min_snr : float
        Minimum peak height in units of the robust noise standard deviation. Zero
        disables the test.
    edge_margin : int
        Pixels at each end of the spectrum in which peaks are ignored.
    min_separation : int
        Minimum distance, in pixels, from a peak to the local minima bracketing it.
    fwhm_min, fwhm_max : float
        Acceptable range of fitted line width, in pixels.

    Returns
    -------
    list[Line]
        Measured lines, ordered by position. Empty if nothing passed the tests.
    """
    prepared, noise = prepare_spectrum(spectrum, smoothing)
    npix = prepared.size

    if npix < 2 * edge_margin + 3:
        return []

    maxima, minima = _local_extrema(prepared)
    if maxima.size == 0:
        return []

    maxima = maxima[(maxima >= edge_margin) & (maxima <= npix - 1 - edge_margin)]
    if maxima.size == 0:
        return []

    # Require a resolved profile: a genuine line has a minimum on each side, far enough
    # away that there are pixels to fit. A side with no minimum at all -- typically
    # because everything between the peak and that edge is flat background, which has
    # no strict extremum of its own -- is still a resolved edge, not an unresolved one;
    # bracket it with a fixed-width window there instead of discarding the peak.
    if minima.size:
        left_of = np.searchsorted(minima, maxima)
        has_left = left_of > 0
        has_right = left_of < minima.size
        left = np.where(has_left, minima[np.clip(left_of - 1, 0, minima.size - 1)],
                        np.maximum(maxima - 5, 0))
        right = np.where(has_right, minima[np.clip(left_of, 0, minima.size - 1)],
                         np.minimum(maxima + 5, npix - 1))
        far_enough = ((maxima - left) >= min_separation) & ((right - maxima) >= min_separation)
        maxima, left, right = maxima[far_enough], left[far_enough], right[far_enough]
    else:
        left = np.maximum(maxima - 5, 0)
        right = np.minimum(maxima + 5, npix - 1)

    if maxima.size == 0:
        return []

    thresholds = [t for t in (_detection_threshold(prepared, cutoff),
                              min_snr * noise if min_snr > 0 else None)
                  if t is not None]
    if thresholds:
        above = prepared[maxima] >= max(thresholds)
        maxima, left, right = maxima[above], left[above], right[above]

        if maxima.size == 0:
            return []

    # Fit only the core of the line. The bracketing minima can lie far away once the
    # spectrum has been smoothed, and fitting a Gaussian across a hundred pixels of
    # baseline is both needless and numerically fragile.
    window = max(int(np.ceil(1.5 * fwhm_max)), min_separation + 1)

    lines = []
    for peak, lo, hi in zip(maxima, left, right):
        lo = max(int(lo), int(peak) - window)
        hi = min(int(hi), int(peak) + window)
        if hi - lo < 3:
            continue

        segment = np.arange(lo, hi + 1, dtype=float)
        try:
            height, centre, variance, _ = fit_gaussian(segment, prepared[lo:hi + 1],
                                                       centre_guess=float(peak))
        except (RuntimeError, ValueError):
            continue

        # Undo the effect of the pre-smoothing on both the width and the height. The
        # reported values must describe the instrument, not this function's own kernel:
        # they feed `QC IFU WAVECAL LINE WIDTH`, which the DRLD ties to the spectral
        # resolution requirement METIS-6073, and `QC IFU WAVECAL PEAK CNTS`.
        # Convolving two Gaussians adds their variances and preserves the area, so the
        # width shrinks in quadrature and the height grows in the same proportion.
        measured_variance = np.abs(variance)
        variance = measured_variance - smoothing ** 2
        if variance <= 0:
            continue

        height *= np.sqrt(measured_variance / variance)

        fwhm = FWHM_PER_SIGMA * np.sqrt(variance)
        if not fwhm_min <= fwhm <= fwhm_max:
            continue
        # A fit that ran away from its segment is not measuring this peak
        if not lo <= centre <= hi:
            continue

        lines.append(Line(position=float(centre), fwhm=float(fwhm),
                          height=float(height), offset=float(offset)))

    return sorted(lines, key=lambda line: line.position)


def _detection_threshold(spectrum: np.ndarray, cutoff: float) -> float | None:
    """Turn the `cutoff` setting into an absolute threshold in counts."""
    if cutoff == 0:
        return None

    if cutoff < 1:
        peak = float(np.max(spectrum))
        return cutoff * peak if peak > 0 else None

    non_zero = spectrum[spectrum != 0]
    if non_zero.size == 0:
        return None

    return float(np.nanpercentile(non_zero, cutoff))
