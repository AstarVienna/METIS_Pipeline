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

Straightening of curved spectral traces, and resampling onto a common wavelength grid.

A trace runs along a curved mid-line across the detector, so the same wavelength falls
on a different row in every column. `rectify_trace` cuts a fixed-height strip that
follows that mid-line, which turns the trace into a rectangle whose rows are constant
cross-dispersion offsets and whose columns are still detector columns.
`resample_to_wavelength_grid` then replaces the column axis by a linear wavelength axis,
which is what the IFU cube products are defined on.

Adapted from PyReduce (`pyreduce.rectify`, `pyreduce.util.make_index`). PyReduce's
`merge_images` is deliberately not ported: it splices overlapping echelle orders onto a
shared wavelength axis, whereas IFU slices are spatially adjacent samples of the field
that must be stacked, not spliced.
"""

import numpy as np
from cpl.core import Msg

from pymetis.drl.trace_model import Trace


def strip_index(centres: np.ndarray,
                height: int,
                first_column: int,
                last_column: int) -> tuple[np.ndarray, np.ndarray]:
    """
    Build the index of a fixed-height strip that follows a varying centre.

    Parameters
    ----------
    centres : np.ndarray
        Cross-dispersion centre of the strip in each column, length `ncol`. Only the
        entries in `[first_column, last_column)` are used.
    height : int
        Strip height in pixels. Must be positive.
    first_column : int
        First column of the strip, inclusive.
    last_column : int
        Last column of the strip, exclusive.

    Returns
    -------
    tuple[np.ndarray, np.ndarray]
        Row and column indices, each of shape `(height, last_column - first_column)`,
        suitable for indexing an image as `image[rows, columns]`.

    Raises
    ------
    ValueError
        If `height` is not positive, or the column range is empty.

    Notes
    -----
    PyReduce builds this with a Python list comprehension per column; broadcasting a
    single `arange` is equivalent and avoids materialising one array per column.
    """
    if height <= 0:
        raise ValueError(f"Expected a positive strip height, but got {height}")
    if last_column <= first_column:
        raise ValueError(
            f"Empty column range [{first_column}, {last_column})"
        )

    columns = np.arange(first_column, last_column)
    # Round rather than truncate: truncation biases the strip downwards by half a pixel
    bottom = np.rint(centres[columns]).astype(int) - height // 2
    rows = bottom[np.newaxis, :] + np.arange(height)[:, np.newaxis]

    return rows, np.broadcast_to(columns, rows.shape)


def rectify_trace(image: np.ndarray,
                  trace: Trace,
                  height: int) -> np.ndarray:
    """
    Cut a straightened, fixed-height strip along a single trace.

    Parameters
    ----------
    image : np.ndarray
        Detector image, `[nrow, ncol]`, dispersion along axis 1.
    trace : Trace
        The trace to straighten. Its `column_range` bounds the strip.
    height : int
        Strip height in pixels.

    Returns
    -------
    np.ndarray
        The strip, `[height, width]`, where `width` is the trace's column range. Row
        `height // 2` holds the trace mid-line. Pixels whose row falls outside the
        detector are set to NaN rather than wrapping around.

    Notes
    -----
    Slit tilt is not applied. PyReduce corrects each strip for slit curvature using
    `Trace.slit`, but `metis_ifu_distortion` does not determine it, so `slit` is always
    `None` here. When it is populated the correction belongs at the end of this
    function.
    """
    nrow, ncol = image.shape
    first_column, last_column = trace.column_range
    first_column = max(0, int(first_column))
    last_column = min(ncol, int(last_column))

    centres = trace.y_at_x(np.arange(ncol))
    rows, columns = strip_index(centres, height, first_column, last_column)

    inside = (rows >= 0) & (rows < nrow)
    strip = np.full(rows.shape, np.nan)
    strip[inside] = image[rows[inside], columns[inside]]

    if not inside.all():
        Msg.debug('rectify_trace',
                  f"Trace {trace.m} runs off the detector in "
                  f"{int((~inside).sum())} of {inside.size} strip pixels")

    return strip


def default_height(traces: list[Trace], fallback: int = 10) -> int:
    """
    Choose a strip height from the spacing of neighbouring traces.

    Parameters
    ----------
    traces : list[Trace]
        Traces to measure. Their mid-lines are evaluated at the detector centre.
    fallback : int
        Height to use when the spacing cannot be measured, i.e. for a single trace.

    Returns
    -------
    int
        A positive, odd strip height, so that the mid-line sits on the centre row.

    Notes
    -----
    `Trace.height` already records a per-trace aperture where one could be derived, but
    a cube needs one height for every slice, so the median spacing is used instead.
    """
    if len(traces) < 2:
        return fallback | 1

    midpoints = np.sort([t.y_at_x(0.0) for t in traces])
    spacing = float(np.median(np.diff(midpoints)))
    height = int(spacing)

    if height < 1:
        Msg.warning('default_height',
                    f"Trace spacing of {spacing:.2f} px is too small to cut a strip; "
                    f"falling back to {fallback}")
        return fallback | 1

    # Odd, so that the mid-line lands on the centre row rather than between two
    return height | 1


def rectify_image(image: np.ndarray,
                  traces: list[Trace],
                  height: int | None = None) -> list[np.ndarray]:
    """
    Straighten every trace in an image.

    Parameters
    ----------
    image : np.ndarray
        Detector image, `[nrow, ncol]`, dispersion along axis 1.
    traces : list[Trace]
        Traces to straighten, as returned by `pymetis.drl.trace.trace` or
        `traces_from_table`.
    height : int, optional
        Strip height in pixels, common to all traces. Derived from the trace spacing
        when unset.

    Returns
    -------
    list[np.ndarray]
        One strip per trace, in the order the traces were given. Empty if `traces` is
        empty. The strips share a height but not necessarily a width, because each
        trace carries its own column range.
    """
    if not traces:
        Msg.warning('rectify_image', "No traces to rectify; returning nothing")
        return []

    if height is None:
        height = default_height(traces)
        Msg.info('rectify_image', f"Strip height, derived from trace spacing: {height}")

    Msg.info('rectify_image',
             f"Rectifying {len(traces)} traces at a height of {height} px")

    return [rectify_trace(image, trace, height) for trace in traces]


def linear_wavelength_grid(wavelengths: np.ndarray,
                           samples: int | None = None) -> np.ndarray:
    """
    Build a linear wavelength axis spanning the wavelengths present.

    Parameters
    ----------
    wavelengths : np.ndarray
        Wavelengths to cover. Non-positive and non-finite entries are ignored, which is
        how `metis_ifu_wavecal` marks pixels without a solution.
    samples : int, optional
        Number of grid points. Defaults to the number of valid wavelengths, which keeps
        the sampling comparable to the detector's.

    Returns
    -------
    np.ndarray
        Monotonically increasing wavelengths, of length `samples`.

    Raises
    ------
    ValueError
        If no valid wavelength is supplied, or they are all identical: neither case
        defines a grid.
    """
    values = np.asarray(wavelengths, dtype=float).ravel()
    valid = np.isfinite(values) & (values > 0)

    if not valid.any():
        raise ValueError(
            "No valid wavelength to build a grid from: every value is non-positive or "
            "not finite. This usually means the wavelength map is empty; check "
            "QC IFU DISTORT NSPOTS on the distortion table it was derived from."
        )

    low, high = float(values[valid].min()), float(values[valid].max())
    if low == high:
        raise ValueError(
            f"All wavelengths are identical ({low}), which does not define a grid"
        )

    return np.linspace(low, high, samples if samples is not None else int(valid.sum()))


def resample_to_wavelength_grid(strip: np.ndarray,
                                wavelengths: np.ndarray,
                                grid: np.ndarray) -> np.ndarray:
    """
    Replace a strip's column axis by a common wavelength axis.

    Parameters
    ----------
    strip : np.ndarray
        Straightened strip, `[height, width]`, as returned by `rectify_trace`.
    wavelengths : np.ndarray
        Wavelength of each column of `strip`, length `width`. Need not be linear, but
        must be monotonic once the invalid entries are dropped.
    grid : np.ndarray
        Target wavelength axis, increasing.

    Returns
    -------
    np.ndarray
        The resampled strip, `[height, len(grid)]`. Grid points outside the strip's
        own wavelength coverage are NaN rather than extrapolated.

    Raises
    ------
    ValueError
        If `wavelengths` does not match the width of `strip`.
    """
    if strip.shape[1] != len(wavelengths):
        raise ValueError(
            f"Strip is {strip.shape[1]} columns wide but {len(wavelengths)} "
            f"wavelengths were given"
        )

    values = np.asarray(wavelengths, dtype=float)
    valid = np.isfinite(values) & (values > 0)

    if valid.sum() < 2:
        Msg.warning('resample_to_wavelength_grid',
                    "Fewer than two columns carry a wavelength; strip dropped")
        return np.full((strip.shape[0], len(grid)), np.nan)

    order = np.argsort(values[valid])
    source = values[valid][order]
    inside = (grid >= source[0]) & (grid <= source[-1])

    resampled = np.full((strip.shape[0], len(grid)), np.nan)
    for row in range(strip.shape[0]):
        samples = strip[row, valid][order]
        # np.interp cannot skip NaN, so drop them per row: a masked pixel in one row
        # should not blank the whole column
        finite = np.isfinite(samples)
        if finite.sum() < 2:
            continue
        resampled[row, inside] = np.interp(grid[inside],
                                           source[finite], samples[finite])

    return resampled


def build_cube(strips: list[np.ndarray],
               wavelengths: list[np.ndarray],
               samples: int | None = None) -> tuple[np.ndarray, np.ndarray]:
    """
    Stack rectified slices into a spectral cube on a common linear wavelength grid.

    Parameters
    ----------
    strips : list[np.ndarray]
        One straightened strip per slice, each `[height, width]`. Heights must agree;
        widths need not.
    wavelengths : list[np.ndarray]
        Wavelength of each column of each strip, matching `strips` element for element.
    samples : int, optional
        Length of the wavelength axis. Defaults to the widest strip, so that the
        best-sampled slice is not degraded.

    Returns
    -------
    tuple[np.ndarray, np.ndarray]
        The wavelength axis, and the cube as `[nslice, height, samples]`. Positions a
        slice does not cover are NaN.

    Raises
    ------
    ValueError
        If no strips are given, if the two lists differ in length, or if the strips
        disagree in height.
    """
    if not strips:
        raise ValueError("No strips to stack into a cube")
    if len(strips) != len(wavelengths):
        raise ValueError(
            f"Got {len(strips)} strips but {len(wavelengths)} wavelength arrays"
        )

    heights = {strip.shape[0] for strip in strips}
    if len(heights) != 1:
        raise ValueError(f"Strips disagree in height: {sorted(heights)}")

    grid = linear_wavelength_grid(
        np.concatenate([np.asarray(w, dtype=float).ravel() for w in wavelengths]),
        samples if samples is not None else max(s.shape[1] for s in strips),
    )

    Msg.info('build_cube',
             f"Stacking {len(strips)} slices onto {len(grid)} wavelength samples "
             f"from {grid[0]:.6g} to {grid[-1]:.6g}")

    cube = np.stack([resample_to_wavelength_grid(strip, waves, grid)
                     for strip, waves in zip(strips, wavelengths)])

    return grid, cube
