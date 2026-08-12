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

Detection of spectral traces by clustering and polynomial fitting.

Adapted from PyReduce (`pyreduce.trace`), which is the Python implementation of the
IDL REDUCE pipeline of Piskunov & Valenti (2002), described in Piskunov, Wehrhahn &
Marquart (2021). The METIS DRLD prescribes exactly these algorithms for IFU
distortion correction (critical algorithm 5b: "Use the same algorithms as for LSS
(PyReduce), but via their incarnation in the CRIRES pipeline"), and describes the
procedure as: smoothing and thresholding the frame in order to distinguish in-order
from inter-order pixels, then fitting continuous clusters of in-order pixels with a
second-degree polynomial.

Original authors: Nikolai Piskunov, Thomas Marquart, Ansgar Wehrhahn (GPLv3).

Differences from upstream PyReduce
----------------------------------
- All plotting and interactive cluster merging is removed. A CPL recipe must never
  block on standard input, so ambiguous merges are resolved by threshold alone.
- Multi-fiber support (fiber grouping, beam pairing, order-centre matching) is
  removed. The METIS IFU has one trace per spatial slice, numbered sequentially
  from the bottom of the detector.
- `trace()` returns an empty list instead of failing when no cluster survives the
  size cuts, so that undersized or unilluminated frames degrade gracefully.
- Per-trace fit residuals are returned, to feed the `QC IFU DISTORT RMS` parameter.
- Logging goes through `cpl.core.Msg` rather than the `logging` module.

Coordinate convention
---------------------
`x` denotes the **column** index (the dispersion coordinate, axis 1) and `y` the
**row** index (the cross-dispersion coordinate, axis 0). Polynomials are fitted as
`y = P(x)`, that is, cross-dispersion position as a function of dispersion position.
The resulting coefficients are stored in `Trace.pos` and evaluated by `Trace.y_at_x`,
which uses the same naming.

Note that upstream PyReduce has `x` and `y` the other way round, fitting `x = P(y)`.
The numerical result is identical -- only the naming differs -- but it does mean this
module can no longer be diffed against upstream line by line.
"""

from functools import cmp_to_key
from itertools import combinations
from typing import Literal

import cpl
import numpy as np
from astropy.convolution import Gaussian2DKernel, interpolate_replace_nans
from cpl.core import Msg
from numpy.polynomial.polynomial import Polynomial
from scipy.ndimage import (binary_closing,
                           binary_opening,
                           gaussian_filter1d,
                           label,
                           median_filter,
                           uniform_filter1d)
from scipy.signal import find_peaks, peak_widths
from scipy.sparse import diags
from scipy.sparse.linalg import spsolve

from pymetis.drl.trace_model import Trace

FilterType = Literal['boxcar', 'gaussian', 'whittaker']


def whittaker_smooth(y: np.ndarray, lam: float, axis: int = 0) -> np.ndarray:
    """
    Whittaker smoother (optimal filter).

    Minimises `sum((y - z)**2) + lam * sum((z[i] - z[i - 1])**2)`, i.e. a least
    squares fit penalised by the first differences of the result. Higher `lam`
    yields a smoother result. Preserves edges better than a boxcar average.

    Parameters
    ----------
    y : np.ndarray
        Input data, 1D or 2D.
    lam : float
        Smoothing parameter.
    axis : int
        Axis along which to smooth, for 2D input.

    Returns
    -------
    np.ndarray
        Smoothed data, same shape as the input.
    """
    if y.ndim == 1:
        n = len(y)
        # Tridiagonal system W + lam * D'D, with D the first-difference matrix
        diag_main = np.ones(n) + 2 * lam
        diag_main[0] = 1 + lam
        diag_main[-1] = 1 + lam
        diag_off = -lam * np.ones(n - 1)
        a = diags([diag_off, diag_main, diag_off], [-1, 0, 1], format='csc')
        return spsolve(a, y)

    return np.apply_along_axis(lambda row: whittaker_smooth(row, lam), axis, y)


def fit(x: np.ndarray, y: np.ndarray, deg: int | Literal['best']) -> np.ndarray:
    """
    Fit `y = P(x)` and return the coefficients in `np.polyval` order.

    Parameters
    ----------
    x : np.ndarray
        Dispersion (column) coordinates of the points to fit.
    y : np.ndarray
        Cross-dispersion (row) coordinates of the points to fit.
    deg : int | 'best'
        Polynomial degree, or `'best'` to select it by an Akaike-like criterion.

    Returns
    -------
    np.ndarray
        Coefficients, highest power first, of shape `(deg + 1,)`. Evaluating them
        with `np.polyval` at a column gives the trace's row, matching `Trace.y_at_x`.
    """
    if deg == 'best':
        return best_fit(x, y)

    # `coef` is ascending, so reverse it for np.polyval order. The copy matters:
    # reversing yields a negative-stride view, and pycpl reads the raw buffer when
    # storing an array into a cpl.core.Table, which would silently write garbage.
    return np.ascontiguousarray(Polynomial.fit(x, y, deg=deg, domain=[]).coef[::-1])


def best_fit(x: np.ndarray, y: np.ndarray) -> np.ndarray:
    """
    Fit `y = P(x)`, choosing the degree that minimises `2 * k + chi squared`.

    Tries degrees 0 to 4 and stops as soon as the criterion worsens.
    """
    aic = np.inf
    coeff = None

    for k in range(5):
        coeff_new = fit(x, y, k)
        chisq = np.sum((np.polyval(coeff_new, x) - y) ** 2)
        aic_new = 2 * k + chisq
        if aic_new > aic:
            break
        coeff = coeff_new
        aic = aic_new

    return coeff


def determine_overlap_rating(xi: np.ndarray,
                             yi: np.ndarray,
                             xj: np.ndarray,
                             yj: np.ndarray,
                             mean_cluster_thickness: float,
                             nrow: int,
                             ncol: int,
                             deg: int | Literal['best'] = 2) -> tuple[float, list[int]]:
    """
    Rate how likely it is that two clusters are parts of the same trace.

    Each cluster is fitted separately, then each fit is extrapolated over the other
    cluster's column span. Where the two curves agree to within the mean cluster
    thickness, the clusters are considered to overlap. The rating is normalised by
    the size of the smaller cluster, which is what limits the accuracy of the fit,
    and penalised by the size of any gap between the two clusters.

    Parameters
    ----------
    xi, yi : np.ndarray
        Dispersion (column) and cross-dispersion (row) coordinates of cluster `i`.
    xj, yj : np.ndarray
        The same for cluster `j`.

    Returns
    -------
    tuple[float, list[int]]
        The overlap rating, and the `[start, end]` column range over which the two
        clusters were found to agree (`-1` where undetermined).
    """
    i_left, i_right = xi.min(), xi.max()
    j_left, j_right = xj.min(), xj.max()

    # The smaller cluster limits how well the fit is constrained
    n_min = min(i_right - i_left, j_right - j_left)

    order_i = fit(xi, yi, deg)
    order_j = fit(xj, yj, deg)

    # Evaluate both polynomials over both clusters' column spans
    y_ii = np.polyval(order_i, np.arange(i_left, i_right))
    y_ij = np.polyval(order_i, np.arange(j_left, j_right))
    y_jj = np.polyval(order_j, np.arange(j_left, j_right))
    y_ji = np.polyval(order_j, np.arange(i_left, i_right))

    diff_i = np.abs(y_ii - y_ji)
    diff_j = np.abs(y_ij - y_jj)

    ind_i = np.where((diff_i < mean_cluster_thickness) & (y_ji >= 0) & (y_ji < nrow))
    ind_j = np.where((diff_j < mean_cluster_thickness) & (y_ij >= 0) & (y_ij < nrow))

    overlap = min(n_min, len(ind_i[0])) + min(n_min, len(ind_j[0]))
    overlap /= 2 * n_min

    # Penalise clusters separated by a gap along the dispersion direction
    if i_right < j_left:
        overlap *= 1 - (i_right - j_left) / ncol
    elif j_right < i_left:
        overlap *= 1 - (j_right - i_left) / ncol

    overlap_region = [-1, -1]
    if len(ind_i[0]) > 0:
        overlap_region[0] = np.min(ind_i[0]) + i_left
    if len(ind_j[0]) > 0:
        overlap_region[1] = np.max(ind_j[0]) + j_left

    return overlap, overlap_region


def calculate_mean_cluster_thickness(x: dict[int, np.ndarray],
                                     y: dict[int, np.ndarray]) -> float:
    """
    Estimate the typical cross-dispersion thickness of a cluster, in pixels.

    `x` and `y` hold the dispersion (column) and cross-dispersion (row) coordinates
    of each cluster's pixels. Used as the tolerance when deciding whether two
    clusters lie on the same trace.
    """
    cluster_thicknesses = []

    for cluster in x.keys():
        columns = x[cluster]
        rows = y[cluster]

        column_thicknesses = []
        for col in np.unique(columns):
            in_col = columns == col
            if np.any(in_col):
                rows_in_col = rows[in_col]
                column_thicknesses.append(rows_in_col.max() - rows_in_col.min())

        if column_thicknesses:
            cluster_thicknesses.append(np.mean(column_thicknesses))

    if not cluster_thicknesses:
        return 10.0

    return 1.5 * np.mean(cluster_thicknesses) / len(cluster_thicknesses)


def create_merge_array(x: dict[int, np.ndarray],
                       y: dict[int, np.ndarray],
                       mean_cluster_thickness: float,
                       nrow: int,
                       ncol: int,
                       deg: int | Literal['best'],
                       threshold: float) -> np.ndarray:
    """
    Rate every pair of clusters and return the candidate merges, best first.

    Returns
    -------
    np.ndarray
        Rows of `[i, j, overlap, region_start, region_end]`, sorted by descending
        overlap, containing only pairs rating above `threshold`.
    """
    cluster_ids = list(x.keys())
    merge = np.zeros((max(len(cluster_ids) ** 2, 1), 5))

    for k, (i, j) in enumerate(combinations(cluster_ids, 2)):
        overlap, region = determine_overlap_rating(
            x[i], y[i], x[j], y[j], mean_cluster_thickness, nrow, ncol, deg=deg,
        )
        merge[k] = [i, j, overlap, *region]

    merge = merge[merge[:, 2] > threshold]
    return merge[np.argsort(merge[:, 2])[::-1]]


def update_merge_array(merge: np.ndarray,
                       x: dict[int, np.ndarray],
                       y: dict[int, np.ndarray],
                       j: int,
                       mean_cluster_thickness: float,
                       nrow: int,
                       ncol: int,
                       deg: int | Literal['best'],
                       threshold: float) -> np.ndarray:
    """Re-rate cluster `j` against all others, after it absorbed another cluster."""
    j = int(j)
    cluster_ids = np.array(list(x.keys()))
    update = []

    for i in cluster_ids[cluster_ids != j]:
        overlap, region = determine_overlap_rating(
            x[i], y[i], x[j], y[j], mean_cluster_thickness, nrow, ncol, deg=deg,
        )
        if overlap <= threshold:
            continue
        update.append([i, j, overlap, *region])

    if not update:
        return merge

    merge = np.concatenate((merge, np.array(update)))
    return merge[np.argsort(merge[:, 2])[::-1]]


def _delete(i: int,
            x: dict[int, np.ndarray],
            y: dict[int, np.ndarray],
            merge: np.ndarray) -> tuple[dict, dict, np.ndarray]:
    """Drop cluster `i` and every candidate merge that referenced it."""
    del x[i], y[i]
    merge = merge[(merge[:, 0] != i) & (merge[:, 1] != i)]
    return x, y, merge


def _combine(i: int,
             j: int,
             x: dict[int, np.ndarray],
             y: dict[int, np.ndarray],
             merge: np.ndarray,
             mean_cluster_thickness: float,
             nrow: int,
             ncol: int,
             deg: int | Literal['best'],
             threshold: float) -> tuple[dict, dict, np.ndarray]:
    """Absorb cluster `i` into cluster `j` and re-rate `j` against the rest."""
    x[j] = np.concatenate((x[j], x[i]))
    y[j] = np.concatenate((y[j], y[i]))

    x, y, merge = _delete(i, x, y, merge)
    merge = merge[(merge[:, 0] != j) & (merge[:, 1] != j)]

    return x, y, update_merge_array(
        merge, x, y, j, mean_cluster_thickness, nrow, ncol, deg, threshold,
    )


def merge_clusters(img: np.ndarray,
                   x: dict[int, np.ndarray],
                   y: dict[int, np.ndarray],
                   deg: int | Literal['best'] = 2,
                   auto_merge_threshold: float = 0.9,
                   merge_min_threshold: float = 0.1) -> tuple[dict, dict, list[int]]:
    """
    Merge clusters that are parts of the same trace, e.g. split by a bad pixel column.

    Unlike upstream PyReduce this is fully non-interactive: pairs rating at or above
    `auto_merge_threshold` are merged and everything else is left alone. Setting
    `auto_merge_threshold` to 1 disables merging altogether and skips the (quadratic)
    rating computation.

    Parameters
    ----------
    img : np.ndarray
        The image the traces are based on. Only its shape is used.
    x, y : dict[int, np.ndarray]
        Dispersion (column) and cross-dispersion (row) coordinates of the pixels of
        each cluster, keyed by cluster id.
    deg : int | 'best'
        Polynomial degree used when rating candidate merges. Kept lower than the
        final fit degree, since a partial cluster constrains a fit poorly.
    auto_merge_threshold : float
        Overlap rating at or above which two clusters are merged.
    merge_min_threshold : float
        Overlap rating below which a pair is not even considered.

    Returns
    -------
    tuple[dict, dict, list[int]]
        The updated coordinate dictionaries and the surviving cluster ids.
    """
    if auto_merge_threshold >= 1:
        return x, y, list(x.keys())

    nrow, ncol = img.shape
    mct = calculate_mean_cluster_thickness(x, y)
    merge = create_merge_array(x, y, mct, nrow, ncol, deg, merge_min_threshold)

    k = 0
    while k < len(merge):
        i, j, overlap, _, _ = merge[k]
        i, j = int(i), int(j)

        if overlap >= auto_merge_threshold:
            Msg.debug('merge_clusters',
                      f"Merging clusters {i} and {j} (overlap {overlap:.3f})")
            x, y, merge = _combine(
                i, j, x, y, merge, mct, nrow, ncol, deg, merge_min_threshold,
            )
        else:
            k += 1

    return x, y, list(x.keys())


def fit_polynomials_to_clusters(x: dict[int, np.ndarray],
                                y: dict[int, np.ndarray],
                                clusters: list[int],
                                degree: int | Literal['best'],
                                ) -> tuple[dict[int, np.ndarray], dict[int, float]]:
    """
    Fit a polynomial mid-line to each cluster.

    Parameters
    ----------
    x, y : dict[int, np.ndarray]
        Dispersion (column) and cross-dispersion (row) coordinates of the pixels of
        each cluster.
    clusters : list[int]
        Cluster ids to fit.
    degree : int | 'best'
        Polynomial degree, or `'best'` to select it per cluster.

    Returns
    -------
    tuple[dict[int, np.ndarray], dict[int, float]]
        Polynomial coefficients per cluster, and the RMS deviation in pixels between
        the measured mid-line and the fitted one.

    Notes
    -----
    The residual deliberately compares the fit against the *per-column centroid* of
    the cluster, not against its individual pixels. Scatter of individual in-order
    pixels measures how thick a trace is, which is a property of the optics and the
    threshold, whereas the DRLD defines `QC IFU DISTORT RMS` as the "root mean square
    deviation between measured position and model".
    """
    coefficients = {c: fit(x[c], y[c], degree) for c in clusters}
    residuals = {c: _centroid_residual(x[c], y[c], coefficients[c]) for c in clusters}
    return coefficients, residuals


def _centroid_residual(x: np.ndarray, y: np.ndarray, coefficients: np.ndarray) -> float:
    """
    RMS deviation, in pixels, of the measured mid-line from the fitted polynomial.

    The measured mid-line is the mean cross-dispersion position of the cluster's
    pixels in each column that the cluster occupies. `x` holds the dispersion
    (column) coordinates and `y` the cross-dispersion (row) ones.
    """
    counts = np.bincount(x)
    sums = np.bincount(x, weights=y)

    occupied = counts > 0
    centroids = sums[occupied] / counts[occupied]
    columns = np.flatnonzero(occupied)

    return float(np.sqrt(np.mean((centroids - np.polyval(coefficients, columns)) ** 2)))


def _split_clusters_by_sigma(im: np.ndarray,
                             x: dict[int, np.ndarray],
                             y: dict[int, np.ndarray],
                             degree_before_merge: int | Literal['best'],
                             min_cluster: int,
                             sigma: float) -> tuple[dict, dict]:
    """
    Split clusters that stray from the global trace shape by more than `sigma`.

    All clusters are shifted to a common baseline and fitted together to establish
    the shape a trace is expected to have. Pixels deviating from that shape by more
    than `sigma` standard deviations are cut from their cluster, and any sufficiently
    large connected component among the cut pixels becomes a new cluster.

    This guards against two adjacent traces being labelled as one connected blob.
    """
    n_before = len(x)

    degrees = {i: 1 if np.sum((np.polyval(np.polyfit(x[i], y[i], 1), x[i]) - y[i]) ** 2)
               < np.sum((np.polyval(np.polyfit(x[i], y[i], 2), x[i]) - y[i]) ** 2)
               else 2
               for i in x.keys()}
    # Constant term per cluster, i.e. its offset from the common shape
    bias = {i: np.polyfit(x[i], y[i], deg=degrees[i])[-1] for i in x.keys()}

    ids = list(x.keys())
    xt = np.concatenate([x[i] for i in ids])
    yt = np.concatenate([y[i] - bias[i] for i in ids])
    deg = 2 if degree_before_merge == 'best' else degree_before_merge
    coef = np.polyfit(xt, yt, deg=deg)

    cutoff = sigma * (np.polyval(coef, xt) - yt).std()
    keep = {
        i: np.abs(np.polyval(coef, x[i]) - (y[i] - bias[i])) < cutoff
        for i in x.keys()
    }

    next_id = max(ids) + 1
    row_idx, col_idx = np.indices(im.shape)

    for i in ids:
        rejected = ~keep[i]

        # Rejected pixels may themselves form a viable trace
        if np.any(rejected):
            new_img = np.zeros(im.shape, dtype=int)
            new_img[y[i][rejected], x[i][rejected]] = 1
            components, n_new = label(new_img)

            for j in range(1, n_new + 1):
                yn = row_idx[components == j]
                if yn.size >= min_cluster:
                    x[next_id] = col_idx[components == j]
                    y[next_id] = yn
                    next_id += 1

        x[i] = x[i][keep[i]]
        y[i] = y[i][keep[i]]
        if len(x[i]) == 0:
            del x[i], y[i]

    if len(x) != n_before:
        Msg.info('trace', f"Sigma clipping: {n_before} -> {len(x)} clusters")

    return x, y


def traces_to_table(traces: list[Trace], degree: int) -> cpl.core.Table:
    """
    Serialise traces into the METIS `IFU_DISTORTION_TABLE` layout.

    One row per trace: `slice_nb` and `trace_nb` identify the trace (`Trace.slice` and
    `Trace.m` respectively), array columns `pos`, `bottom`, and `top` hold the
    `degree + 1` coefficients of the trace mid-line and edges in `np.polyval` order.
    An array column `column_range` holds the first and last valid dispersion coordinate.
    This is the layout `IfuDistortionTable.read()` expects and that `metis_ifu_rsrf` and
    `metis_ifu_wavecal` consume.

    Parameters
    ----------
    traces : list[Trace]
        Traces to serialise. May be empty, in which case an empty table with the
        correct columns is returned.
    degree : int
        Polynomial degree of the mid-line fits, which fixes the width of `pos`.

    Returns
    -------
    cpl.core.Table
        The distortion table for a single detector.

    Raises
    ------
    ValueError
        If any trace has a coefficient count inconsistent with `degree`.

    Notes
    -----
    Arrays are forced contiguous before being stored. pycpl reads the raw buffer of a
    numpy array when assigning it into a `cpl.core.Table` array column, so handing it a
    strided view (such as a reversed one) silently writes uninitialised memory instead
    of the intended values.
    """
    table = cpl.core.Table.empty(len(traces))
    table.new_column('slice_nb', cpl.core.Type.INT)
    table.new_column('trace_nb', cpl.core.Type.INT)
    table.new_column_array('column_range', cpl.core.Type.DOUBLE, 2)
    table.new_column_array('pos', cpl.core.Type.DOUBLE, degree + 1)
    # Measured illuminated extent, so consumers need not guess it back from the trace
    # spacing. NaN marks a row whose edges were never measured -- the best available
    # placeholder, even though it does not survive a real FITS round trip: CPL's table
    # reader materialises an invalid array element as 0.0 on load.
    table.new_column_array('bottom', cpl.core.Type.DOUBLE, degree + 1)
    table.new_column_array('top', cpl.core.Type.DOUBLE, degree + 1)

    unmeasured = np.full(degree + 1, np.nan)

    for row, t in enumerate(traces):
        if len(t.pos) != degree + 1:
            raise ValueError(f"Trace {t.m} has {len(t.pos)} coefficients, "
                             f"expected {degree + 1} for degree {degree}")

        for name, coefficients in (('bottom', t.bottom), ('top', t.top)):
            if coefficients is not None and len(coefficients) != degree + 1:
                raise ValueError(
                    f"Trace {t.m} has a {name} edge of {len(coefficients)} coefficients, "
                    f"expected {degree + 1} for degree {degree}"
                )

        table['slice_nb', row] = int(t.slice) if t.slice is not None else int(t.m)
        table['trace_nb', row] = int(t.m)
        table['pos', row] = np.ascontiguousarray(t.pos, dtype=float)
        table['column_range', row] = np.ascontiguousarray(t.column_range, dtype=float)
        table['bottom', row] = np.ascontiguousarray(
            unmeasured if t.bottom is None else t.bottom, dtype=float)
        table['top', row] = np.ascontiguousarray(
            unmeasured if t.top is None else t.top, dtype=float)

    return table


def traces_from_table(table: cpl.core.Table) -> list[Trace]:
    """
    Reconstruct traces from an `IFU_DISTORTION_TABLE` extension.

    The inverse of `traces_to_table`.

    Parameters
    ----------
    table : cpl.core.Table
        A single detector's extension of the distortion table.

    Returns
    -------
    list[Trace]
        Traces ordered as stored, with `m`/`slice` read from the `trace_nb`/`slice_nb`
        columns.
    """
    if len(table) == 0:
        return []

    trace_nbs = np.asarray(table.column_array('trace_nb')[0]).ravel()
    slice_nbs = np.asarray(table.column_array('slice_nb')[0]).ravel()
    column_ranges = np.asarray(table.column_array('column_range')[0], dtype=float)
    coefficients = np.asarray(table.column_array('pos')[0], dtype=float)
    bottom = np.asarray(table.column_array('bottom')[0], dtype=float)
    top = np.asarray(table.column_array('top')[0], dtype=float)

    traces = [
        Trace(m=int(trace_nbs[row]),
              slice=int(slice_nbs[row]),
              pos=np.ascontiguousarray(coefficients[row]),
              column_range=(int(column_ranges[row][0]), int(column_ranges[row][1])),
              bottom=np.ascontiguousarray(bottom[row]),
              top=np.ascontiguousarray(top[row]))
        for row in range(len(table))
    ]

    for t in traces:
        if t.has_edges:
            mid = 0.5 * (t.column_range[0] + t.column_range[1])
            t.height = float(t.height_at_x(mid))

    return traces


def compute_heights(traces: list[Trace], ncol: int) -> None:
    """
    Set the extraction aperture height of each trace, in place.

    The height is the largest distance to the nearest neighbouring trace, measured at
    nine reference columns spread across the detector. Traces with no neighbour keep
    a height of `None`.
    """
    ntrace = len(traces)
    if ntrace < 2:
        return

    ref_cols = (np.linspace(0.1, 0.9, 9) * ncol).astype(int)

    for i, t in enumerate(traces):
        valid_cols = ref_cols[(ref_cols >= t.column_range[0])
                              & (ref_cols < t.column_range[1])]
        if len(valid_cols) == 0:
            valid_cols = [(t.column_range[0] + t.column_range[1]) // 2]

        max_height = 0.0
        for x in valid_cols:
            y_i = t.y_at_x(x)

            if i == 0:
                height = abs(traces[i + 1].y_at_x(x) - y_i)
            elif i == ntrace - 1:
                height = abs(y_i - traces[i - 1].y_at_x(x))
            else:
                height = min(y_i - traces[i - 1].y_at_x(x),
                             traces[i + 1].y_at_x(x) - y_i)

            max_height = max(max_height, height)

        t.height = max_height


def measure_trace_fwhm(im: np.ndarray,
                       traces: list[Trace],
                       half_window: int = 25) -> float | None:
    """
    Measure the median cross-dispersion FWHM of a set of traces, in pixels.

    For the METIS IFU this quantifies how sharply the image slicer's slices are
    imaged onto the detector, which the DRLD notes "gives an indication of the
    variation of spectral resolution across the field of view". It feeds the
    `QC IFU DISTORT FWHM` quality control parameter.

    The profile is measured on the image itself rather than on the thresholded
    clusters, whose extent depends on the detection threshold rather than on the
    optics.

    The width of each trace is taken where its profile falls to half way between its
    peak and the surrounding inter-trace background. That definition holds for a narrow
    pinhole spot and for a broad flat-topped band alike, which matters because the same
    code sees both: a pinhole exposure gives compact spots, while a continuum-illuminated
    frame gives slices tens of pixels tall with a noisy plateau. `scipy.signal.peak_widths`
    is deliberately not used here, since it measures down from the peak by a fraction of
    the *prominence* and so latches onto any small bump on such a plateau.

    Parameters
    ----------
    im : np.ndarray
        The image the traces were detected in.
    traces : list[Trace]
        Traces to measure.
    half_window : int
        Half-width, in columns, of the band around the detector centre over which the
        cross-dispersion profile is taken.

    Returns
    -------
    float | None
        Median FWHM in pixels, or `None` if no trace could be measured.
    """
    if not traces:
        return None

    nrow, ncol = im.shape
    mid = ncol // 2
    columns = slice(max(mid - half_window, 0), min(mid + half_window, ncol))

    # A median across neighbouring columns suppresses noise without smearing the
    # profile, since the traces are near-horizontal over such a narrow band.
    profile = np.median(np.asarray(im, dtype=float)[:, columns], axis=1)

    centres = [t.y_at_x(mid) for t in traces
               if t.column_range[0] <= mid < t.column_range[1]]
    centres = [c for c in centres if 0 <= c < nrow]

    if not centres:
        return None

    widths = []
    for i, centre in enumerate(centres):
        reach = _measurement_reach(centres, i, traces[0].height if traces else None)
        width = _half_maximum_width(profile, centre, reach)
        if width is not None:
            widths.append(width)

    if not widths:
        return None

    return float(np.median(widths))


def measure_trace_edges(im: np.ndarray,
                        traces: list[Trace],
                        degree: int | Literal['best'] | None = None,
                        n_bands: int = 9,
                        half_window: int = 25,
                        min_bands: int = 3) -> None:
    """
    Measure the illuminated extent of each trace from the image and fit its edges.

    Sets `Trace.bottom`, `Trace.top` and `Trace.height` in place. This is the
    measurement the DRLD implies for the extraction aperture: the edges come from where
    the cross-dispersion profile falls to half its height above the local background,
    rather than from the spacing to the neighbouring traces, which is all
    `compute_heights` can offer once the image is gone.

    Parameters
    ----------
    im : np.ndarray
        The image the traces were detected in, `[nrow, ncol]`.
    traces : list[Trace]
        Traces to measure. Modified in place; those whose edges cannot be measured keep
        `bottom`/`top` of `None`.
    degree : int | 'best', optional
        Polynomial degree of the edge fits. Defaults to the degree of each trace's own
        mid-line, since an edge curves much as the centre does.
    n_bands : int
        Number of column bands spread across each trace's valid column range.
    half_window : int
        Half-width, in columns, of each band. As in `measure_trace_fwhm`, a median over
        neighbouring columns suppresses noise without smearing a near-horizontal trace.
    min_bands : int
        Fewest successful bands that will still be fitted. A polynomial of degree `d`
        needs `d + 1` points, and this is checked against that too.

    Notes
    -----
    The reach of each measurement is bounded by `_measurement_reach`, i.e. half the
    distance to the nearest neighbour, so an edge fit cannot wander into the adjacent
    slice even where two slices nearly touch.
    """
    if not traces:
        return

    nrow, ncol = im.shape
    image = np.asarray(im, dtype=float)

    # Profiles are cached per band, since every trace is measured in the same bands
    band_centres = np.unique(np.clip((np.linspace(0.1, 0.9, n_bands) * ncol).astype(int),
                                     0, ncol - 1))
    profiles = {}
    for x in band_centres:
        columns = slice(max(int(x) - half_window, 0), min(int(x) + half_window, ncol))
        profiles[int(x)] = np.median(image[:, columns], axis=1)

    for i, t in enumerate(traces):
        xs, bottoms, tops = [], [], []

        for x in band_centres:
            if not (t.column_range[0] <= x < t.column_range[1]):
                continue

            # Neighbour spacing is evaluated at this column, not at the detector centre
            centres = [other.y_at_x(x) for other in traces]
            centre = centres[i]
            if not 0 <= centre < nrow:
                continue

            edges = _half_maximum_edges(profiles[int(x)], centre,
                                        _measurement_reach(centres, i, t.height))
            if edges is None:
                continue

            xs.append(float(x))
            bottoms.append(edges[0])
            tops.append(edges[1])

        deg = t.degree if degree is None else degree
        needed = max(min_bands, (deg + 1) if isinstance(deg, int) else 1)

        if len(xs) < needed:
            Msg.debug('measure_trace_edges',
                      f"Trace {t.m}: only {len(xs)} of {len(band_centres)} bands gave a "
                      f"bounded profile, need {needed}; edges left unmeasured")
            continue

        x_arr = np.asarray(xs)
        t.bottom = fit(x_arr, np.asarray(bottoms), deg)
        t.top = fit(x_arr, np.asarray(tops), deg)
        # Evaluated at the midpoint of the column range rather than averaged over the
        # bands, so that a trace read back from the table reports the same height as the
        # one that was written -- `traces_from_table` has only the polynomials to work
        # from and derives the height the same way.
        t.height = float(t.height_at_x(0.5 * (t.column_range[0] + t.column_range[1])))

    measured = sum(1 for t in traces if t.has_edges)
    Msg.info('measure_trace_edges',
             f"Measured edges for {measured} of {len(traces)} traces")


def _measurement_reach(centres: list[float], index: int, height: float | None) -> int:
    """
    How far either side of a trace to look when measuring its width, in pixels.

    Half the distance to the closest neighbour, so that the window reaches the
    inter-trace minimum but never crosses into the next trace.
    """
    distances = [abs(centres[j] - centres[index])
                 for j in (index - 1, index + 1)
                 if 0 <= j < len(centres)]

    if distances:
        return max(int(round(0.5 * min(distances))), 2)

    # A lone trace has no neighbour to bound it, so fall back on its own aperture
    return max(int(round(height)) if height else 20, 2)


def _half_maximum_edges(profile: np.ndarray,
                        centre: float,
                        reach: int) -> tuple[float, float] | None:
    """
    Positions where a feature falls to half its height above the local background.

    Parameters
    ----------
    profile : np.ndarray
        Cross-dispersion profile.
    centre : float
        Approximate centre of the feature.
    reach : int
        How far either side of `centre` to search.

    Returns
    -------
    tuple[float, float] | None
        The lower and upper crossing, in cross-dispersion pixels, or `None` if the
        profile does not fall to the half level on both sides within `reach`, which
        means the feature is not bounded by the window and no extent can be attributed
        to it.
    """
    low = max(int(np.floor(centre)) - reach, 0)
    high = min(int(np.ceil(centre)) + reach + 1, profile.size)
    window = profile[low:high]

    if window.size < 3:
        return None

    peak = low + int(np.argmax(window))
    background = float(window.min())
    half = background + 0.5 * (profile[peak] - background)

    if not profile[peak] > background:
        return None

    def crossing(step: int) -> float | None:
        """Interpolated position where the profile drops below `half` going outwards."""
        position = peak
        while low <= position + step < high:
            nxt = position + step
            if profile[nxt] < half:
                # Linear interpolation between the bracketing samples
                span = profile[position] - profile[nxt]
                fraction = (profile[position] - half) / span if span else 0.0
                return position + step * fraction
            position = nxt
        return None

    bottom, top = crossing(-1), crossing(+1)
    if bottom is None or top is None:
        return None

    return float(bottom), float(top)


def _half_maximum_width(profile: np.ndarray,
                        centre: float,
                        reach: int) -> float | None:
    """
    Width of a feature at half its height above the local background, in pixels.

    A thin wrapper over `_half_maximum_edges`, which locates the two crossings this
    width is the distance between.
    """
    edges = _half_maximum_edges(profile, centre, reach)
    return None if edges is None else float(edges[1] - edges[0])


def trace(im: np.ndarray,
          *,
          min_cluster: int | None = None,
          min_width: int | float | None = None,
          filter_x: int = 0,
          filter_y: int | None = None,
          filter_type: FilterType = 'boxcar',
          noise: float = 0,
          noise_relative: float = 0,
          degree: int | Literal['best'] = 4,
          degree_before_merge: int | Literal['best'] = 2,
          border_width: int | list[int] | None = None,
          closing_shape: tuple[int, int] = (5, 5),
          opening_shape: tuple[int, int] = (2, 2),
          auto_merge_threshold: float = 0.9,
          merge_min_threshold: float = 0.1,
          sigma: float = 0) -> list[Trace]:
    """
    Detect spectral traces in an image and fit a polynomial mid-line to each.

    Implements the DRLD prescription for IFU distortion: smooth and threshold the
    frame to distinguish in-order from inter-order pixels, then fit continuous
    clusters of in-order pixels with a polynomial.

    The steps are: estimate any unset parameters from the image; interpolate over
    masked pixels; estimate the local background by smoothing along the
    cross-dispersion direction; threshold against that background; discard the
    detector borders; close gaps and remove specks morphologically; label connected
    components and discard those that are too small or too narrow; optionally split
    clusters that stray from the common trace shape; merge clusters that belong to
    the same trace; fit each surviving cluster; and sort the results from the bottom
    of the detector upwards.

    Parameters
    ----------
    im : np.ndarray
        The image to trace, `[nrow, ncol]`, dispersion running along axis 1. May be
        a masked array, in which case masked pixels are interpolated over.
    min_cluster : int, optional
        Smallest acceptable cluster, in pixels. Defaults to `ncol // 4`.
    min_width : int | float, optional
        Smallest acceptable cluster extent along the dispersion direction. A float is
        interpreted as a fraction of the detector width. Defaults to `0.25`. Zero
        disables the check.
    filter_x : int
        Smoothing width along the dispersion direction, applied before thresholding.
        Zero disables it. Useful for noisy data or thin traces.
    filter_y : int, optional
        Smoothing width along the cross-dispersion direction, used to estimate the
        local background. Estimated from the trace spacing if unset. Must be positive.
    filter_type : {'boxcar', 'gaussian', 'whittaker'}
        Smoothing kernel. `whittaker` preserves edges best, `boxcar` is cheapest.
    noise : float
        Absolute threshold above the local background.
    noise_relative : float
        Relative threshold, as a fraction of the local background. If both `noise`
        and `noise_relative` are zero, a relative threshold of 0.1% is used.
    degree : int | 'best'
        Degree of the final per-trace polynomial fit. The DRLD prescribes 2 for the
        METIS IFU.
    degree_before_merge : int | 'best'
        Degree used while rating candidate merges, where fits are poorly constrained.
    border_width : int | list[int], optional
        Pixels to ignore at the detector edges, either uniformly or as
        `[top, bottom, left, right]`. Estimated from the trace width if unset.
    closing_shape : tuple[int, int]
        Structuring element used to close gaps within a trace.
    opening_shape : tuple[int, int]
        Structuring element used to remove isolated specks.
    auto_merge_threshold : float
        Overlap rating at or above which two clusters are merged. 1 disables merging.
    merge_min_threshold : float
        Overlap rating below which a pair is not considered for merging.
    sigma : float
        If positive, split clusters deviating from the common trace shape by more
        than this many standard deviations. Zero disables the check.

    Returns
    -------
    list[Trace]
        One `Trace` per detected trace, ordered from the bottom of the detector
        upwards with `m` assigned sequentially from zero. Empty if nothing was
        detected -- which is the expected outcome for an unilluminated frame, or for
        one smaller than `min_cluster`.
    """
    # Work in signed integers to avoid underflow when subtracting the background
    im = np.asanyarray(im)
    im = im.astype(int)
    nrow, ncol = im.shape

    if filter_y is None:
        col = im[:, nrow // 2]
        col = median_filter(col, 5)
        npeaks = find_peaks(col, height=np.percentile(col, 90))[0].size
        if npeaks == 0:
            Msg.warning('trace',
                        "Could not estimate the cross-dispersion filter size: "
                        "no peaks found in the central column. No traces detected.")
            return []
        filter_y = nrow // (npeaks * 2)
        Msg.info('trace', f"Cross-dispersion filter size, estimated: {filter_y}")
    if filter_y <= 0:
        raise ValueError(f"Expected filter_y > 0, but got {filter_y}")

    if border_width is None:
        # Estimate from the width of the brightest trace in the central column
        col = median_filter(im[:, nrow // 2], 5)
        width = peak_widths(col, [int(np.argmax(col))])[0][0]
        border_width = int(np.ceil(width))
        Msg.info('trace', f"Image border width, estimated: {border_width}")

    if isinstance(border_width, (list, tuple)):
        if len(border_width) != 4:
            raise ValueError("border_width list must have 4 elements "
                             f"[top, bottom, left, right], got {len(border_width)}")
        bw_top, bw_bottom, bw_left, bw_right = (int(b) for b in border_width)
        if any(b < 0 for b in (bw_top, bw_bottom, bw_left, bw_right)):
            raise ValueError(f"All border_width values must be >= 0, got {border_width}")
    elif isinstance(border_width, (int, float, np.integer, np.floating)):
        bw = int(border_width)
        if bw < 0:
            raise ValueError(f"Expected border_width >= 0, but got {bw}")
        bw_top = bw_bottom = bw_left = bw_right = bw
    else:
        raise TypeError("border_width must be int or list of 4 int, "
                        f"got {type(border_width)}")

    if min_cluster is None:
        min_cluster = ncol // 4
        Msg.info('trace', f"Minimum cluster size, estimated: {min_cluster}")
    elif not np.isscalar(min_cluster):
        raise TypeError(f"Expected scalar minimum cluster size, but got {min_cluster}")

    if min_width is None:
        min_width = 0.25
    if isinstance(min_width, (float, np.floating)):
        # A fraction of the extent along the dispersion direction.
        # NOTE: upstream PyReduce scales by im.shape[0] here; that is only equivalent
        # for square detectors, and min_width is compared against a column extent.
        min_width = int(min_width * ncol)

    if filter_type not in ('boxcar', 'gaussian', 'whittaker'):
        raise ValueError("filter_type must be one of "
                         f"('boxcar', 'gaussian', 'whittaker'), got {filter_type}")

    # Interpolate over masked pixels so they do not punch holes in the clusters
    if np.ma.is_masked(im):
        im_clean = np.ma.filled(im.astype(float), fill_value=np.nan)
        kernel = Gaussian2DKernel(x_stddev=1.5, y_stddev=2.5)
        im_clean = np.asarray(interpolate_replace_nans(im_clean, kernel))
        im_clean = np.nan_to_num(im_clean, nan=0.0)
    else:
        im_clean = np.asarray(im, dtype=float)

    match filter_type:
        case 'boxcar':
            def smooth(data, size, axis):
                return uniform_filter1d(data, int(size), axis=axis, mode='nearest')
        case 'gaussian':
            def smooth(data, size, axis):
                return gaussian_filter1d(data, size, axis=axis)
        case _:
            def smooth(data, size, axis):
                return whittaker_smooth(data, size, axis=axis)

    # Smooth along the dispersion direction, so only cross-dispersion structure remains
    if filter_x > 0:
        im_clean = smooth(im_clean, filter_x, axis=1)

    background = smooth(im_clean, filter_y, axis=0)

    if noise == 0 and noise_relative == 0:
        noise_relative = 0.001
        Msg.info('trace', "Using default noise_relative=0.001 (0.1% of background)")

    # In-order pixels are those standing above the local background
    mask = im_clean > background * (1 + noise_relative) + noise

    if bw_top > 0:
        mask[:bw_top, :] = False
    if bw_bottom > 0:
        mask[-bw_bottom:, :] = False
    if bw_left > 0:
        mask[:, :bw_left] = False
    if bw_right > 0:
        mask[:, -bw_right:] = False

    mask = np.ma.filled(mask, fill_value=False)
    # Close gaps within a trace, then remove isolated specks
    mask = binary_closing(mask, np.full(closing_shape, 1), border_value=1)
    mask = binary_opening(mask, np.full(opening_shape, 1))

    clusters, n_initial = label(mask)
    Msg.info('trace', f"Found {n_initial} clusters initially")

    # Discard clusters too small to constrain a fit
    sizes = np.bincount(clusters.ravel())
    big_enough = sizes > min_cluster
    big_enough[0] = True                    # index 0 is the background
    n_too_small = int(np.sum(~big_enough)) - 1
    clusters[~big_enough[clusters]] = 0

    ids = np.unique(clusters)
    ids = ids[ids != 0]
    # x is the dispersion (column, axis 1) coordinate throughout, y the
    # cross-dispersion (row, axis 0) one, so that every fit reads as y = P(x)
    x = {i: np.where(clusters == c)[1] for i, c in enumerate(ids)}
    y = {i: np.where(clusters == c)[0] for i, c in enumerate(ids)}

    if n_too_small > 0:
        Msg.info('trace',
                 f"Removed {n_too_small} clusters smaller than "
                 f"min_cluster={min_cluster}, {len(x)} remain")

    if not x:
        Msg.warning('trace',
                    f"No cluster survived the minimum size cut (min_cluster="
                    f"{min_cluster}, largest found {int(sizes[1:].max()) if len(sizes) > 1 else 0}"
                    f" px). No traces detected.")
        return []

    if sigma > 0:
        x, y = _split_clusters_by_sigma(
            im, x, y, degree_before_merge, min_cluster, sigma,
        )
        if not x:
            Msg.warning('trace', "No cluster survived sigma clipping. "
                                 "No traces detected.")
            return []

    x, y, ids = merge_clusters(
        im, x, y,
        deg=degree_before_merge,
        auto_merge_threshold=auto_merge_threshold,
        merge_min_threshold=merge_min_threshold,
    )

    # Discard clusters spanning too little of the dispersion direction
    if min_width > 0:
        n_before = len(x)
        for k in [k for k, v in x.items() if v.max() - v.min() <= min_width]:
            del x[k], y[k]
        ids = list(x.keys())

        if n_before - len(x) > 0:
            Msg.info('trace',
                     f"Removed {n_before - len(x)} clusters narrower than "
                     f"min_width={min_width}, {len(x)} remain")

        if not x:
            Msg.warning('trace', "No cluster survived the minimum width cut. "
                                 "No traces detected.")
            return []

    Msg.info('trace', f"Fitting polynomials to {len(x)} clusters")
    coefficients, residuals = fit_polynomials_to_clusters(x, y, ids, degree)

    # Sort from the bottom of the detector upwards. Traces are compared over the
    # columns they share, since a mean over disjoint spans is not meaningful.
    def compare(i, j):
        _, yi, i_left, i_right = i
        _, yj, j_left, j_right = j

        if i_right < j_left or j_right < i_left:
            return yi.mean() - yj.mean()

        left, right = max(i_left, j_left), min(i_right, j_right)
        return yi[left:right].mean() - yj[left:right].mean()

    columns = np.arange(ncol)
    keys = sorted(
        [(c, np.polyval(coefficients[c], columns), x[c].min(), x[c].max())
         for c in x.keys()],
        key=cmp_to_key(compare),
    )

    traces = [
        Trace(m=m,
              slice=m,
              pos=coefficients[cluster_id],
              column_range=(int(x[cluster_id].min()), int(x[cluster_id].max()) + 1),
              residual=residuals[cluster_id])
        for m, (cluster_id, _, _, _) in enumerate(keys, start=1)
    ]

    compute_heights(traces, ncol)

    Msg.info('trace', f"Detected {len(traces)} traces")
    return traces
