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
low-order polynomials". Emission lines are measured at several cross-dispersion offsets
within each slice, so that the fit captures the tilt of the lines with respect to the
detector columns without needing a separate slit tilt determination.

The line detection and Gaussian centroiding it builds on are adapted from PyReduce; see
`pymetis.drl.lines`. The offset-spectrum extraction follows the same approach PyReduce
uses for slit curvature (`pyreduce.slit_curve.Curvature._extract_offset_spectra`).
"""

from dataclasses import dataclass, field

import numpy as np
from cpl.core import Msg

from pymetis.drl.lines import Line, detect_lines
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
    coefficients : np.ndarray | None
        Coefficients of `lambda(x, dy)`, where `x` is the dispersion coordinate in
        pixels and `dy` the signed cross-dispersion distance from the slice mid-line,
        also in pixels. `coeff[i, j]` multiplies `x**i * dy**j`. `None` if no solution
        could be established at all.
    degree : tuple[int, int]
        Degrees actually used, which may be lower than requested if the number of
        identified lines or offsets could not constrain the full polynomial.
    lines : list[Line]
        Every line measured in this slice, with `wavelength` set on those identified.
    rms : float | None
        Root mean square residual of the fit, in the same unit as the wavelengths.
    fallback : bool
        True if the solution came from the supplied approximate model rather than from
        measured lines.
    """

    index: int
    coefficients: np.ndarray | None
    degree: tuple[int, int]
    lines: list[Line] = field(default_factory=list)
    rms: float | None = None
    fallback: bool = False

    @property
    def n_identified(self) -> int:
        """Number of lines that were assigned a wavelength."""
        return sum(1 for line in self.lines if line.wavelength is not None)

    def evaluate(self, x: np.ndarray, dy: np.ndarray) -> np.ndarray | None:
        """Evaluate the solution at dispersion coordinate `x` and offset `dy`."""
        if self.coefficients is None:
            return None

        return polyval2d_safe(x, dy, self.coefficients)


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

    return spectra, offsets


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


def fit_wavelength_solution(lines: list[Line],
                            degree: tuple[int, int],
                            ) -> tuple[np.ndarray | None, float | None, tuple[int, int]]:
    """
    Fit `lambda(x, dy)` to identified lines, reducing the degree where unconstrained.

    The dispersion degree cannot exceed one less than the number of distinct
    wavelengths, and the cross-dispersion degree one less than the number of distinct
    offsets. Requesting more would fit noise, or fail outright, so both are clamped.

    Returns
    -------
    tuple[np.ndarray | None, float | None, tuple[int, int]]
        Coefficients, RMS residual, and the degrees actually used. Coefficients are
        `None` if there was nothing to fit.
    """
    identified = [line for line in lines if line.wavelength is not None]
    if not identified:
        return None, None, (0, 0)

    x = np.array([line.position for line in identified], dtype=float)
    dy = np.array([line.offset for line in identified], dtype=float)
    z = np.array([line.wavelength for line in identified], dtype=float)

    n_wavelengths = np.unique(z).size
    n_offsets = np.unique(dy).size

    degree_x = min(int(degree[0]), max(n_wavelengths - 1, 0))
    degree_y = min(int(degree[1]), max(n_offsets - 1, 0))

    if degree_x < 1:
        # A single wavelength fixes an offset but says nothing about dispersion
        return None, None, (degree_x, degree_y)

    if (degree_x, degree_y) != (int(degree[0]), int(degree[1])):
        Msg.info('fit_wavelength_solution',
                 f"Reduced solution degree from {tuple(degree)} to "
                 f"({degree_x}, {degree_y}): {n_wavelengths} distinct wavelengths at "
                 f"{n_offsets} offsets")

    try:
        coefficients = polyfit2d(x, dy, z, degree=(degree_x, degree_y))
    except (ValueError, np.linalg.LinAlgError) as exc:
        Msg.warning('fit_wavelength_solution', f"Wavelength solution failed: {exc}")
        return None, None, (degree_x, degree_y)

    residuals = polyval2d_safe(x, dy, coefficients) - z
    rms = float(np.sqrt(np.mean(np.square(residuals))))

    return coefficients, rms, (degree_x, degree_y)


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

    Extracts spectra at several cross-dispersion offsets, measures the emission lines in
    each, groups them, identifies them against the expected wavelengths, and fits
    `lambda(x, dy)`. Falls back to `approximate` if identification or fitting fails.

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
        Requested degrees in the dispersion and cross-dispersion directions.
    n_offsets : int
        Number of offset spectra to extract.
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
    spectra, offsets = extract_offset_spectra(image, trace, height=height,
                                              n_offsets=n_offsets)

    detections: list[Line] = []
    for spectrum, offset in zip(spectra, offsets):
        if spectrum.count() == 0:
            continue
        detections.extend(detect_lines(spectrum, offset=float(offset), **detect_options))

    groups = group_lines(detections, tolerance=group_tolerance)
    assign_wavelengths(groups, wavelengths,
                       approximate=approximate, tolerance=match_tolerance)

    lines = [line for group in groups for line in group]
    coefficients, rms, degree_used = fit_wavelength_solution(lines, degree)

    if coefficients is None:
        return SliceSolution(index=trace.m if trace.m is not None else 0,
                             coefficients=approximate,
                             degree=(1, 0) if approximate is not None else (0, 0),
                             lines=lines,
                             rms=None,
                             fallback=approximate is not None)

    return SliceSolution(index=trace.m if trace.m is not None else 0,
                         coefficients=coefficients,
                         degree=degree_used,
                         lines=lines,
                         rms=rms,
                         fallback=False)


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
        if solution.coefficients is None:
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
