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

Two-dimensional wavelength solution for an integral field spectrograph.

Implements the DRLD prescription for the METIS IFU: for each spatial slice `i`, a
wavelength solution `lambda = g_i(x, y)`, "sufficiently accurately described by
low-order polynomials", split into two independent fits. The tilt of a line's position
with cross-dispersion offset is fit first, from every line detected in several offset
spectra regardless of whether it was identified -- the tilt needs no wavelength at all.
The wavelength solution is fit second, from lines identified in a single, high-
signal-to-noise spectrum obtained by collapsing the whole slice height, correcting for
the tilt just fit so a tilted line is summed along its own ridge rather than smeared.

The line detection and Gaussian centroiding it builds on are adapted from PyReduce; see
`pymetis.drl.lines`. The offset-spectrum extraction used for the tilt fit follows the
same approach PyReduce uses for slit curvature
(`pyreduce.slit_curve.Curvature._extract_offset_spectra`); the tilt-corrected collapse
used for the wavelength fit follows PyReduce's curvature-corrected extraction instead
(`pyreduce.extract.simple_extraction`/`correct_for_curvature`).
"""

from dataclasses import dataclass, field

import cpl
import numpy as np
from astropy.io import fits
from cpl.core import Msg

from pymetis.drl.lines import Line, detect_lines
from pymetis.drl.rectify import rectify_trace
from pymetis.drl.trace import traces_from_table, traces_to_table
from pymetis.drl.trace_model import Trace
from pymetis.engine.core.functions.polyfit2d import polyfit2d, polyval2d_safe


@dataclass
class SliceSolution:
    """
    The wavelength solution for a single spatial slice.

    Attributes
    ----------
    index : int
        Slice number, matching `Trace.m`.
    wavelength_coefficients : np.ndarray | None
        Coefficients of `lambda(x)`, the wavelength as a polynomial in dispersion
        position `x` at offset zero (1D, low-order-first). `None` if no solution could
        be established at all.
    tilt_coefficients : np.ndarray | None
        Coefficients describing how a line's position shifts away from its offset-zero
        position as a function of both. `coeff[i, j]` multiplies `x**i * dy**j`; column
        0 (the `dy**0` term) is identically zero, since a line has no shift at its own
        reference offset. `None` if the offsets could not constrain any tilt.
    degree : tuple[int, int]
        (dispersion, tilt) degrees actually used, which may be lower than requested if
        the number of identified lines or offsets could not constrain the full
        polynomial.
    lines : list[Line]
        Every line measured in the collapsed, tilt-corrected spectrum, with
        `wavelength` set on those identified.
    rms : float | None
        Root mean square residual of the wavelength fit, in the same unit as the
        wavelengths.
    fallback : bool
        True if the wavelength solution came from the supplied approximate model rather
        than from measured lines.
    """

    index: int
    wavelength_coefficients: np.ndarray | None
    tilt_coefficients: np.ndarray | None
    degree: tuple[int, int]
    lines: list[Line] = field(default_factory=list)
    rms: float | None = None
    fallback: bool = False
    trace: Trace | None = None

    @property
    def n_identified(self) -> int:
        """Number of lines that were assigned a wavelength."""
        return sum(1 for line in self.lines if line.wavelength is not None)

    def evaluate(self, x: np.ndarray, dy: np.ndarray) -> np.ndarray | None:
        """Evaluate the solution at dispersion coordinate `x` and offset `dy`."""
        return _evaluate_solution(x, dy, self.wavelength_coefficients,
                                  self.tilt_coefficients)


def _evaluate_solution(x: np.ndarray,
                       dy: np.ndarray,
                       wavelength_coefficients: np.ndarray | None,
                       tilt_coefficients: np.ndarray | None) -> np.ndarray | None:
    """
    Wavelength at dispersion position `x`, offset `dy` from the slice mid-line.

    `wavelength_coefficients` is fitted as a function of a line's position at offset
    zero, which `x` alone is not -- a line's position drifts from it by the tilt as
    `dy` moves away from zero. The offset-zero position is recovered by evaluating the
    fitted tilt at the pixel's own `x` rather than solving for it self-consistently: the
    tilt is a small correction, so the error this introduces is second order.
    """
    if wavelength_coefficients is None:
        return None

    x = np.asarray(x, dtype=float)
    dy = np.asarray(dy, dtype=float)

    if tilt_coefficients is not None:
        x = x - polyval2d_safe(x, dy, tilt_coefficients)

    return np.polynomial.polynomial.polyval(x, wavelength_coefficients)


def linear_solution(wavelength_start: float,
                    wavelength_end: float,
                    ncol: int) -> np.ndarray:
    """
    Coefficients of a purely linear dispersion, with no cross-dispersion dependence.

    Used as the fallback when too few lines are identified to fit a real solution, and
    as the approximate model against which detected lines are identified.
    """
    coefficients = np.zeros((2, 1))
    coefficients[0, 0] = wavelength_start
    coefficients[1, 0] = (wavelength_end - wavelength_start) / max(ncol - 1, 1)
    return coefficients


def extract_offset_spectra(image: np.ndarray,
                           trace: Trace,
                           *,
                           height: float,
                           n_offsets: int = 5) -> tuple[np.ndarray, np.ndarray]:
    """
    Extract several 1D spectra along a slice, at different cross-dispersion offsets.

    Each spectrum follows the curved mid-line of the slice, displaced by a fixed offset,
    and is median-collapsed over a band of rows. Measuring the same line at several
    offsets is what makes the two-dimensional fit possible: the shift of a line's
    centroid with offset *is* the tilt.

    Parameters
    ----------
    image : np.ndarray
        Detector image, `[nrow, ncol]`.
    trace : Trace
        The slice to extract along.
    height : float
        Full height of the slice in pixels, over which the offsets are spread.
    n_offsets : int
        Number of spectra to extract. Must be at least 1; the cross-dispersion degree of
        the solution is limited to `n_offsets - 1`.

    Returns
    -------
    tuple[np.ndarray, np.ndarray]
        A masked array of shape `(n_offsets, ncol)` and the offsets in pixels. Columns
        outside the trace's valid range, and offsets that would fall off the detector,
        are masked.
    """
    nrow, ncol = image.shape
    start, end = trace.column_range

    columns = np.arange(ncol)
    centre = np.polyval(trace.pos, columns)

    n_offsets = max(int(n_offsets), 1)
    # Confine the offsets to the inner part of the slice, away from its edges where the
    # illumination falls off and the profile is no longer representative
    span = 0.8 * height
    band = max(int(round(span / n_offsets)), 1)
    offsets = (np.arange(n_offsets) - (n_offsets - 1) / 2) * (span / n_offsets)

    spectra = np.ma.masked_all((n_offsets, ncol))
    half = band // 2
    band_rows = np.arange(band)[:, None]

    for i, offset in enumerate(offsets):
        bottom = np.round(centre + offset).astype(int) - half
        top = bottom + band - 1

        # Gather the whole curved band at once: rows[b, c] is the b-th row of the band
        # in column c. Out-of-range columns are excluded rather than clipped, so that a
        # band running off the detector is reported as missing instead of truncated.
        valid = (bottom >= 0) & (top < nrow)
        valid[:start] = False
        valid[end:] = False
        columns_here = np.flatnonzero(valid)
        if columns_here.size == 0:
            continue

        rows_here = bottom[columns_here][None, :] + band_rows
        spectra[i, columns_here] = np.median(image[rows_here, columns_here[None, :]],
                                            axis=0)

    # # DEBUG: dump the extracted offset spectra for inspection. Remove once done.
    # debug_hdus = [fits.PrimaryHDU()]
    # for i, offset in enumerate(offsets):
    #     debug_hdu = fits.ImageHDU(
    #         data=np.ma.filled(spectra[i], np.nan).astype(np.float32), name=f"OFFSET_{i}")
    #     debug_hdu.header['OFFSET'] = (float(offset), 'cross-dispersion offset, pixels')
    #     debug_hdus.append(debug_hdu)
    # fits.HDUList(debug_hdus).writeto(
    #     f"./debug_{trace.m}.fits", overwrite=True)

    return spectra, offsets


def extract_collapsed_spectrum(image: np.ndarray,
                               trace: Trace,
                               *,
                               height: float,
                               tilt_coefficients: np.ndarray | None = None,
                               ) -> np.ma.MaskedArray:
    """
    Collapse the whole slice height into one high-signal-to-noise 1D spectrum.

    Adapted from PyReduce's curvature-corrected extraction (`pyreduce.extract.
    simple_extraction`/`correct_for_curvature`): each row of the rectified strip is
    resampled onto the offset-zero dispersion grid using the fitted tilt before the
    rows are collapsed, so a tilted line is summed along its own ridge rather than
    smeared across columns. This is the extraction the wavelength fit uses -- unlike
    `extract_offset_spectra`, which only samples thin bands at fixed offsets to
    *measure* the tilt in the first place.

    Falls back to a plain median collapse, uncorrected, when no tilt model is
    available (still better signal-to-noise than a thin band, just not tilt-corrected).

    Parameters
    ----------
    image : np.ndarray
        Detector image.
    trace : Trace
        The slice to extract.
    height : float
        Full height of the slice in pixels.
    tilt_coefficients : np.ndarray, optional
        As fitted by `fit_tilt_solution`. `None` skips the curvature correction.

    Returns
    -------
    np.ma.MaskedArray
        1D spectrum, shape `(ncol,)`, masked outside `trace.column_range` and wherever
        the correction had nothing valid to interpolate from.
    """
    ncol = image.shape[1]
    int_height = max(int(round(height)), 1)
    # `rectify_trace`'s strip is only as wide as `trace.column_range` and is indexed
    # from its start, not from the detector's column 0 -- the tilt model and the
    # returned spectrum are both in absolute detector columns, so that offset has to be
    # carried explicitly throughout.
    strip = rectify_trace(image, trace, int_height)          # (int_height, width)
    start = max(0, int(trace.column_range[0]))
    end = min(ncol, int(trace.column_range[1]))
    columns = np.arange(start, end, dtype=float)

    if tilt_coefficients is not None:
        rows = np.arange(int_height) - int_height // 2       # offsets from the mid-line
        corrected = np.full_like(strip, np.nan)
        for i, dy in enumerate(rows):
            shift = polyval2d_safe(columns, np.full_like(columns, float(dy)),
                                   tilt_coefficients)
            valid = ~np.isnan(strip[i])
            if valid.sum() < 2:
                continue
            corrected[i] = np.interp(columns + shift, columns[valid], strip[i][valid],
                                     left=np.nan, right=np.nan)
        strip = corrected

    collapsed = np.ma.median(np.ma.masked_invalid(strip), axis=0)
    spectrum = np.ma.masked_all(ncol)
    spectrum[start:end] = collapsed
    return spectrum


def group_lines(lines: list[Line], tolerance: float = 3.0) -> list[list[Line]]:
    """
    Group detections that are the same physical line seen at different offsets.

    Parameters
    ----------
    lines : list[Line]
        Detections from every offset spectrum of one slice.
    tolerance : float
        Maximum shift in pixels between consecutive detections of the same line. Must
        exceed the total tilt across the slice, or a tilted line will be split in two.

    Returns
    -------
    list[list[Line]]
        Groups, ordered by mean position, each containing at most one line per offset.
    """
    groups: list[list[Line]] = []

    for line in sorted(lines, key=lambda item: item.position):
        for group in groups:
            mean = np.mean([member.position for member in group])
            taken = {member.offset for member in group}
            if abs(line.position - mean) <= tolerance and line.offset not in taken:
                group.append(line)
                break
        else:
            groups.append([line])

    return sorted(groups, key=lambda group: np.mean([m.position for m in group]))


def assign_wavelengths(groups: list[list[Line]],
                       wavelengths: list[float],
                       *,
                       approximate: np.ndarray | None = None,
                       tolerance: float | None = None) -> int:
    """
    Attach a wavelength to each group of detections, in place.

    Two strategies, in order of preference:

    1. If an approximate solution is available, predict where each expected wavelength
       should fall and match it to the nearest group within `tolerance` pixels. This is
       the robust path, and the one the DRLD implies when it speaks of computing the
       deviation from the optical model.
    2. Otherwise, if the number of groups equals the number of expected wavelengths,
       match them in order. Dispersion is monotonic, so sorting by position and by
       wavelength gives the same sequence, up to the sign of the dispersion.

    These are alternatives, not a cascade: when a model is supplied and rejects every
    candidate, nothing is assigned. Falling back to the ordering there would relabel
    lines the model has just ruled out.

    If neither applies, nothing is assigned and the caller must fall back.

    Returns
    -------
    int
        The number of groups that were assigned a wavelength.
    """
    if not groups or not wavelengths:
        return 0

    positions = np.array([np.mean([m.position for m in g]) for g in groups])
    expected = np.sort(np.asarray(wavelengths, dtype=float))

    Msg.info('assign_wavelengths',
             f"Attempting to assign {len(expected)} expected wavelengths to "
                f"{len(groups)} detected lines")

    Msg.info('assign_wavelengths',
             f"Detected line positions: {positions}")
    Msg.info('assign_wavelengths',
                f"Expected wavelengths: {expected}")

    if approximate is not None:
        predicted = polyval2d_safe(positions, np.zeros_like(positions), approximate)

        if tolerance is not None and expected.size > 1:
            separation = np.min(np.diff(expected))
            if 2 * tolerance > separation:
                Msg.warning('assign_wavelengths',
                            f"The match tolerance {tolerance} exceeds half the closest "
                            f"spacing between expected wavelengths ({separation}); "
                            f"lines that close together cannot be told apart")

        # Invert the approximate model numerically. Every plausible pairing of a detected
        # line with an expected wavelength is scored, then the closest pairings are taken
        # first, each line and each wavelength being used at most once. Matching greedily
        # in wavelength order instead would let a distant wavelength claim a line that
        # another wavelength fits far better.
        pairs = []
        for group_idx, wavelength_prediction in enumerate(np.atleast_1d(predicted)):
            for wavelength in expected:
                distance = abs(float(wavelength_prediction) - float(wavelength))
                if tolerance is None or distance <= tolerance:
                    pairs.append((distance, group_idx, float(wavelength)))

        assigned = 0
        used_groups, used_wavelengths = set(), set()
        for distance, group_idx, wavelength in sorted(pairs):
            if group_idx in used_groups or wavelength in used_wavelengths:
                continue
            for member in groups[group_idx]:
                member.wavelength = wavelength
            used_groups.add(group_idx)
            used_wavelengths.add(wavelength)
            assigned += 1

        if assigned == 0:
            Msg.warning('assign_wavelengths',
                        f"None of the {len(groups)} detected lines lies within "
                        f"{tolerance} of an expected wavelength according to the "
                        f"approximate model; no line was identified")

        # Deliberately no fall-through to positional matching. The model is better
        # information than the ordering, so if it rejects every candidate then the
        # detections are not the expected lines and must not be relabelled as such.
        return assigned

    if len(groups) == len(expected):
        for group, wavelength in zip(groups, expected):
            for member in group:
                member.wavelength = float(wavelength)
        return len(groups)

    return 0


#: Hard cap on both the per-group dx=f(dy) degree and each tilt coefficient's own
#: degree in x, independent of the recipe's dispersion-degree parameter. Matches the
#: fixed 3-coefficient `slit_poly_a/b/c` layout `solutions_to_table` serialises this
#: into (the CRIRES+ `trace_wave` convention for the same PyReduce slit-curvature
#: algorithm).
MAX_TILT_DEGREE = 2


def fit_tilt_solution(groups: list[list[Line]],
                      degree_spatial: int,
                      ) -> tuple[np.ndarray | None, tuple[int, int]]:
    """
    Fit the tilt of a line's position with cross-dispersion offset.

    Uses every detected line in every group, whether or not it was identified with a
    wavelength -- the tilt depends only on how a line's position shifts with offset,
    which needs no wavelength at all, and using every detection rather than only the
    (usually much smaller) identified subset gives the fit far more to work with.

    For each group, fits `position = Q(offset)`; `Q`'s constant term is the group's
    reference position at offset zero, and `Q` minus that constant is exactly the
    `dx = f(dy)` shift of the line away from it. Fitting each of those coefficients as
    a function of the groups' reference positions then gives the tilt as a function of
    dispersion position too. Both degrees -- `Q`'s own, and each coefficient's fit
    across the slice -- are capped at `MAX_TILT_DEGREE`, independent of `degree_spatial`
    and of the recipe's dispersion-degree parameter respectively.

    Parameters
    ----------
    groups : list[list[Line]]
        Detections of the same physical line across offsets, as returned by
        `group_lines`. Lines need not be identified with a wavelength.
    degree_spatial : int
        Requested degree of `Q` (position as a function of offset). Reduced where the
        data cannot constrain it, and in any case capped at `MAX_TILT_DEGREE`.

    Returns
    -------
    tuple[np.ndarray | None, tuple[int, int]]
        Tilt coefficients (2D, `coeff[i, j]` multiplies `x**i * dy**j`; column 0 is
        identically zero since a line has no shift at its own reference offset), and
        the (dispersion, cross-dispersion) degrees actually used. `None` if no group
        had enough distinct offsets to fit any tilt at all.
    """
    references, tilts, group_degrees_y = [], [], []
    for group in groups:
        if not group:
            continue

        offsets = np.array([line.offset for line in group], dtype=float)
        positions = np.array([line.position for line in group], dtype=float)
        group_degree_y = min(int(degree_spatial), MAX_TILT_DEGREE,
                             np.unique(offsets).size - 1)

        fit = polyfit2d(offsets, np.zeros_like(offsets), positions,
                        degree=(group_degree_y, 0))[:, 0]
        references.append(fit[0])
        tilt = fit.copy()
        tilt[0] = 0.0
        tilts.append(tilt)
        group_degrees_y.append(group_degree_y)

    if not references:
        return None, (0, 0)

    degree_y = min(group_degrees_y)
    if degree_y != int(degree_spatial):
        Msg.info('fit_tilt_solution',
                 f"Reduced the tilt degree from {degree_spatial} to {degree_y}: some "
                 f"group has too few distinct offsets, or {MAX_TILT_DEGREE} is the "
                 f"most this model supports")

    if degree_y < 1:
        return None, (0, degree_y)

    references = np.array(references)
    degree_x = min(MAX_TILT_DEGREE, len(references) - 1)

    columns = [np.zeros(degree_x + 1)]
    for j in range(1, degree_y + 1):
        values = np.array([tilt[j] for tilt in tilts])
        columns.append(polyfit2d(references, np.zeros_like(references), values,
                                 degree=(degree_x, 0))[:, 0])

    return np.column_stack(columns), (degree_x, degree_y)


def fit_wavelength_solution(lines: list[Line],
                            degree: int,
                            ) -> tuple[np.ndarray | None, float | None, int]:
    """
    Fit wavelength = P(x) to identified lines from the collapsed, tilt-corrected spectrum.

    A plain 1D fit: the collapsed spectrum's lines already sit at their offset-zero
    position by construction of the extraction, so no cross-dispersion term belongs
    here -- that's `fit_tilt_solution`'s job, on a different, larger set of detections.

    Returns
    -------
    tuple[np.ndarray | None, float | None, int]
        Coefficients (1D, low-order-first), RMS residual, and the degree actually used.
        `None` coefficients if there was nothing to fit.
    """
    identified = [line for line in lines if line.wavelength is not None]
    if not identified:
        return None, None, 0

    x = np.array([line.position for line in identified], dtype=float)
    z = np.array([line.wavelength for line in identified], dtype=float)

    n_wavelengths = np.unique(z).size
    degree_used = min(int(degree), max(n_wavelengths - 1, 0))

    if degree_used < 1:
        # A single wavelength fixes a position but says nothing about dispersion
        return None, None, degree_used

    if degree_used != int(degree):
        Msg.info('fit_wavelength_solution',
                 f"Reduced the dispersion degree from {degree} to {degree_used}: "
                 f"only {n_wavelengths} distinct wavelengths identified")

    try:
        coefficients = polyfit2d(x, np.zeros_like(x), z, degree=(degree_used, 0))[:, 0]
    except (ValueError, np.linalg.LinAlgError) as exc:
        Msg.warning('fit_wavelength_solution', f"Wavelength solution failed: {exc}")
        return None, None, degree_used

    residuals = np.polynomial.polynomial.polyval(x, coefficients) - z
    rms = float(np.sqrt(np.mean(np.square(residuals))))

    return coefficients, rms, degree_used


def solve_slice(image: np.ndarray,
                trace: Trace,
                *,
                wavelengths: list[float],
                height: float,
                degree: tuple[int, int] = (2, 1),
                n_offsets: int = 5,
                approximate: np.ndarray | None = None,
                match_tolerance: float | None = None,
                group_tolerance: float = 3.0,
                **detect_options) -> SliceSolution:
    """
    Determine the wavelength solution for one spatial slice.

    Two independent passes. First, the tilt: spectra are extracted at several
    cross-dispersion offsets, every line detected in them (identified or not) is
    grouped across offsets, and the tilt of position with offset is fit from that.
    Second, the wavelength solution: the whole slice height is collapsed into one
    high-signal-to-noise spectrum, correcting for the tilt just fit, lines in it are
    identified against the expected wavelengths, and `lambda(x)` is fit to those. Falls
    back to `approximate` if identification or fitting fails; the tilt, being
    independent, is kept regardless.

    Parameters
    ----------
    image : np.ndarray
        Detector image.
    trace : Trace
        The slice, from the distortion table.
    wavelengths : list[float]
        Expected line wavelengths, in the unit the solution should be expressed in.
    height : float
        Slice height in pixels.
    degree : tuple[int, int]
        Requested degrees in the dispersion and cross-dispersion (tilt) directions.
    n_offsets : int
        Number of offset spectra to extract for the tilt fit.
    approximate : np.ndarray, optional
        Approximate solution, used both to identify lines and as the fallback.
    match_tolerance : float, optional
        Maximum wavelength difference when matching a line to an expected wavelength.
    group_tolerance : float
        Maximum pixel shift between detections of the same line at different offsets.
    **detect_options
        Passed to `pymetis.drl.lines.detect_lines`.

    Returns
    -------
    SliceSolution
    """
    # Pass 1: tilt, from every detection at every offset (identified or not)
    spectra, offsets = extract_offset_spectra(image, trace, height=height,
                                              n_offsets=n_offsets)

    Msg.info('solve_slice',
             f"Extracted {spectra.shape[0]} offset spectra for slice {trace.m if trace.m is not None else 0}")
    detections: list[Line] = []
    for spectrum, offset in zip(spectra, offsets):
        if spectrum.count() == 0:
            continue
        detections.extend(detect_lines(spectrum, offset=float(offset), **detect_options))

    Msg.info('solve_slice',
             f"Detected {len(detections)} lines in offset spectra for slice {trace.m if trace.m is not None else 0}")

    tilt_groups = group_lines(detections, tolerance=group_tolerance)
    tilt_coefficients, tilt_degree = fit_tilt_solution(tilt_groups, degree[1])

    # Pass 2: wavelength, from one tilt-corrected, full-height collapsed spectrum
    collapsed = extract_collapsed_spectrum(image, trace, height=height,
                                           tilt_coefficients=tilt_coefficients)
    lines = ([] if collapsed.count() == 0
            else detect_lines(collapsed, offset=0.0, **detect_options))

    groups = [[line] for line in lines]
    assign_wavelengths(groups, wavelengths,
                       approximate=approximate, tolerance=match_tolerance)

    wavelength_coefficients, rms, dispersion_degree = fit_wavelength_solution(
        lines, degree[0])

    if wavelength_coefficients is None:
        fallback = (None if approximate is None
                   else np.asarray(approximate, dtype=float).reshape(-1))
        return SliceSolution(index=trace.m if trace.m is not None else 0,
                             wavelength_coefficients=fallback,
                             tilt_coefficients=tilt_coefficients,
                             degree=(1 if approximate is not None else 0,
                                    tilt_degree[1]),
                             lines=lines,
                             rms=None,
                             fallback=approximate is not None,
                             trace=trace)

    return SliceSolution(index=trace.m if trace.m is not None else 0,
                         wavelength_coefficients=wavelength_coefficients,
                         tilt_coefficients=tilt_coefficients,
                         degree=(dispersion_degree, tilt_degree[1]),
                         lines=lines,
                         rms=rms,
                         fallback=False,
                         trace=trace)

def build_wavelength_map(shape: tuple[int, int],
                         traces: list[Trace],
                         solutions: list[SliceSolution],
                         heights: list[float]) -> np.ndarray:
    """
    Paint the per-slice solutions into a wavelength image.

    Pixels not covered by any slice are left at exactly zero. That is part of the
    product contract: `metis_ifu_rsrf` treats zero as "no wavelength" when building its
    blackbody image, and rejects those pixels.

    Parameters
    ----------
    shape : tuple[int, int]
        Shape of the detector image.
    traces : list[Trace]
        Slices, in the same order as `solutions` and `heights`.
    solutions : list[SliceSolution]
        Wavelength solution per slice. Slices whose solution has no coefficients are
        skipped, leaving their pixels at zero.
    heights : list[float]
        Height in pixels of each slice.

    Returns
    -------
    np.ndarray
        The wavelength map, in whatever unit the solutions were fitted in.
    """
    nrow, ncol = shape
    wavelength_map = np.zeros(shape, dtype=float)
    rows = np.arange(nrow)

    for trace, solution, height in zip(traces, solutions, heights):
        if solution.wavelength_coefficients is None:
            continue

        start, end = trace.column_range
        columns = np.arange(max(start, 0), min(end, ncol))
        if columns.size == 0:
            continue

        centre = np.polyval(trace.pos, columns)
        half = height / 2.0

        # Signed distance of every pixel in these columns from the slice mid-line
        offset_grid = rows[:, None] - centre[None, :]
        inside = np.abs(offset_grid) <= half
        if not np.any(inside):
            continue

        row_idx, col_idx = np.nonzero(inside)
        wavelength_map[row_idx, columns[col_idx]] = solution.evaluate(
            columns[col_idx].astype(float), offset_grid[row_idx, col_idx],
        )

    return wavelength_map


def solutions_to_table(solutions: list[SliceSolution]) -> cpl.core.Table:
    """
    Serialise per-slice wavelength solutions into the `IFU_WAVECAL_TAB` layout.

    One row per slice, holding the flattened wavelength and tilt coefficients together
    with the information the wavelength map cannot carry: how well the fit did, how
    many lines it rested on, and whether it is a real fit at all.

    Parameters
    ----------
    solutions : list[SliceSolution]
        Solutions to serialise, in slice order. May be empty, in which case an empty
        table with the correct columns is returned.

    Returns
    -------
    cpl.core.Table
        The wavelength solution table for a single detector.

    Notes
    -----
    `wavelength_coefficients` is stored flattened in C order, with its shape recorded
    in `degree_dispersion`, since a CPL array column is one-dimensional. Its width is
    fixed by the largest solution present, so that slices which fell back to a lower
    degree still fit; unused entries are NaN.

    The tilt is instead stored as three fixed-width-3 columns, `slit_poly_a`/`b`/`c` --
    the `coeff[i, j]` multiplying `x**i * dy**j` for `j = 0, 1, 2` respectively -- since
    `fit_tilt_solution` caps both the tilt-vs-offset degree and each coefficient's own
    degree in `x` at `MAX_TILT_DEGREE` (2), matching the CRIRES+ `trace_wave` file
    convention for the same PyReduce slit-curvature algorithm. `slit_poly_a` is always
    identically zero whenever a tilt was fit at all (see `fit_tilt_solution`); slots
    beyond the degree actually achieved -- or all of them, if no tilt was fit -- are
    NaN, not zero, so a genuine zero coefficient stays distinguishable from one that was
    simply never fit.

    `solution.trace`, when present, is stored using exactly `traces_to_table`'s own
    `slice_nb`/`trace_nb`/`column_range`/`pos`/`bottom`/`top` columns, so the table
    doubles as an `IFU_DISTORTION_TABLE` for its detector. A solution with no trace
    leaves `pos`/`bottom`/`top`/`column_range` NaN; `slice_nb`/`trace_nb` still record
    `solution.index`, which is also how `solutions_from_table` recovers `index` for
    such a row.

    The `fallback` column is the point of the product. A slice that fell back on the
    approximate dispersion model produces a perfectly ordinary-looking wavelength map,
    and until now that fact survived only as a log warning.
    """
    wavelength_width = max((s.wavelength_coefficients.size for s in solutions
                           if s.wavelength_coefficients is not None), default=1)
    trace_degree = max((s.trace.degree for s in solutions if s.trace is not None),
                       default=0)

    table = cpl.core.Table.empty(len(solutions))
    table.new_column('slice_nb', cpl.core.Type.INT)
    table.new_column('trace_nb', cpl.core.Type.INT)
    table.new_column_array('column_range', cpl.core.Type.DOUBLE, 2)
    table.new_column_array('pos', cpl.core.Type.DOUBLE, trace_degree + 1)
    table.new_column_array('bottom', cpl.core.Type.DOUBLE, trace_degree + 1)
    table.new_column_array('top', cpl.core.Type.DOUBLE, trace_degree + 1)
    table.new_column('degree_dispersion', cpl.core.Type.INT)
    table.new_column('degree_spatial', cpl.core.Type.INT)
    table.new_column('n_lines', cpl.core.Type.INT)
    table.new_column('n_identified', cpl.core.Type.INT)
    table.new_column('rms', cpl.core.Type.DOUBLE)
    table.new_column('fallback', cpl.core.Type.INT)
    table.new_column_array('wavelength_coefficients', cpl.core.Type.DOUBLE,
                           wavelength_width)
    table.new_column_array('slit_poly_a', cpl.core.Type.DOUBLE, MAX_TILT_DEGREE + 1)
    table.new_column_array('slit_poly_b', cpl.core.Type.DOUBLE, MAX_TILT_DEGREE + 1)
    table.new_column_array('slit_poly_c', cpl.core.Type.DOUBLE, MAX_TILT_DEGREE + 1)

    unmeasured_trace = np.full(trace_degree + 1, np.nan)
    traced_rows = [row for row, s in enumerate(solutions) if s.trace is not None]
    if traced_rows:
        trace_table = traces_to_table([solutions[row].trace for row in traced_rows],
                                      trace_degree)
        for table_row, row in enumerate(traced_rows):
            for name in trace_table.column_names:
                table[name, row] = trace_table[name, table_row][0]

    for row, solution in enumerate(solutions):
        if solution.trace is None:
            table['slice_nb', row] = int(solution.index)
            table['trace_nb', row] = int(solution.index)
            table['column_range', row] = np.full(2, np.nan)
            table['pos', row] = unmeasured_trace
            table['bottom', row] = unmeasured_trace
            table['top', row] = unmeasured_trace

        wavelength_padded = np.full(wavelength_width, np.nan)
        if solution.wavelength_coefficients is not None:
            flat = np.asarray(solution.wavelength_coefficients, dtype=float).ravel()
            wavelength_padded[:flat.size] = flat

        tilt = solution.tilt_coefficients
        for name, j in (('slit_poly_a', 0), ('slit_poly_b', 1), ('slit_poly_c', 2)):
            column = np.full(MAX_TILT_DEGREE + 1, np.nan)
            if tilt is not None and j < tilt.shape[1]:
                column[:tilt.shape[0]] = tilt[:, j]
            table[name, row] = np.ascontiguousarray(column, dtype=float)

        table['degree_dispersion', row] = int(solution.degree[0])
        table['degree_spatial', row] = int(solution.degree[1])
        table['n_lines', row] = int(len(solution.lines))
        table['n_identified', row] = int(solution.n_identified)
        table['rms', row] = float('nan') if solution.rms is None else float(solution.rms)
        # CPL tables have no boolean column type
        table['fallback', row] = int(solution.fallback)
        table['wavelength_coefficients', row] = np.ascontiguousarray(
            wavelength_padded, dtype=float)

    return table


def solutions_from_table(table: cpl.core.Table) -> list[SliceSolution]:
    """
    Reconstruct wavelength solutions from an `IFU_WAVECAL_TAB` extension.

    The inverse of `solutions_to_table`, with one exception: `lines`. The table stores
    only the `n_lines`/`n_identified` *counts*, not the individual `Line` measurements
    (`position`, `wavelength`, `height`, `fwhm`) they were computed from, so there is
    nothing to reconstruct them from. Every returned `SliceSolution.lines` is therefore
    empty, and `.n_identified` -- a property computed from `lines` -- reads `0`
    regardless of what the table's own `n_identified` column says. A caller that needs
    those counts should read `table['n_lines', ...]`/`table['n_identified', ...]`
    directly instead of going through this function.

    `trace` comes from `traces_from_table`, which already knows to read back `None`
    for a row with no trace at all (its `pos` column is entirely NaN, as
    `solutions_to_table` writes for such a solution) rather than a placeholder `Trace`.

    Parameters
    ----------
    table : cpl.core.Table
        A single detector's extension of the wavelength solution table.

    Returns
    -------
    list[SliceSolution]
        Solutions ordered as stored.
    """
    if len(table) == 0:
        return []

    wavelength_columns = np.asarray(table.column_array('wavelength_coefficients')[0],
                                    dtype=float)
    slit_poly = {
        name: np.asarray(table.column_array(name)[0], dtype=float)
        for name in ('slit_poly_a', 'slit_poly_b', 'slit_poly_c')
    }
    # `trace_nb` also carries `index` for a solution with no trace at all --
    # `solutions_to_table` writes it there either way.
    trace_nbs = np.asarray(table.column_array('trace_nb')[0]).ravel()
    degree_dispersion = np.asarray(table.column_array('degree_dispersion')[0]).ravel()
    degree_spatial = np.asarray(table.column_array('degree_spatial')[0]).ravel()
    rms = np.asarray(table.column_array('rms')[0]).ravel()
    fallback = np.asarray(table.column_array('fallback')[0]).ravel()
    traces = traces_from_table(table)

    solutions = []
    for row in range(len(table)):
        wavelength_row = wavelength_columns[row]
        valid = ~np.isnan(wavelength_row)
        wavelength_coefficients = (np.ascontiguousarray(wavelength_row[valid])
                                  if valid.any() else None)

        # `slit_poly_a` is always fully populated (with zeros, not NaN) up to
        # `degree_x + 1` rows whenever any tilt was fit at all -- it is what fixes the
        # shared width for whichever of `slit_poly_b`/`slit_poly_c` were also fit.
        a = slit_poly['slit_poly_a'][row]
        if np.isnan(a).all():
            tilt_coefficients = None
        else:
            width = int(np.sum(~np.isnan(a)))
            columns = [a[:width], slit_poly['slit_poly_b'][row][:width]]
            c = slit_poly['slit_poly_c'][row]
            if not np.isnan(c).all():
                columns.append(c[:width])
            tilt_coefficients = np.column_stack(columns)

        solutions.append(SliceSolution(
            index=int(trace_nbs[row]),
            wavelength_coefficients=wavelength_coefficients,
            tilt_coefficients=tilt_coefficients,
            degree=(int(degree_dispersion[row]), int(degree_spatial[row])),
            rms=None if np.isnan(rms[row]) else float(rms[row]),
            fallback=bool(fallback[row]),
            trace=traces[row],
        ))

    return solutions
