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

Bivariate polynomial fitting, the two-dimensional counterpart of `polyfit.py`.

Adapted from PyReduce (`pyreduce.util.polyfit2d`), whose wavelength calibration the
METIS DRLD prescribes for the IFU. Used to fit the DRLD's per-slice wavelength solution
`lambda = g_i(x, y)`.

Original authors: Nikolai Piskunov, Thomas Marquart, Ansgar Wehrhahn (GPLv3).
"""

import numpy as np
from numpy.polynomial.polynomial import polyval2d
from scipy.linalg import lstsq
from scipy.special import binom


def _coefficient_indices(coefficients: np.ndarray) -> np.ndarray:
    """Return the `(i, j)` index pairs of a coefficient array, in raster order."""
    idx = np.indices(coefficients.shape)
    return idx.T.swapaxes(0, 1).reshape((-1, 2))


def _scale(x: np.ndarray, y: np.ndarray) -> tuple:
    """
    Shift and scale `x` and `y` to zero mean and unit variance.

    Keeps the Vandermonde matrix well conditioned, which matters as soon as the
    coordinates are large (detector pixel indices reach into the thousands, and their
    fourth power overflows the useful precision of a least squares solve).
    """
    offset_x, offset_y = np.mean(x), np.mean(y)
    norm_x, norm_y = np.std(x), np.std(y)

    # A degenerate coordinate (all points at the same x or y) must not divide by zero
    norm_x = norm_x if norm_x != 0 else 1.0
    norm_y = norm_y if norm_y != 0 else 1.0

    return ((x - offset_x) / norm_x,
            (y - offset_y) / norm_y,
            (norm_x, norm_y),
            (offset_x, offset_y))


def polyscale2d(coefficients: np.ndarray,
                scale_x: float,
                scale_y: float,
                copy: bool = True) -> np.ndarray:
    """Rewrite coefficients of `P(x/scale_x, y/scale_y)` as coefficients of `P(x, y)`."""
    if copy:
        coefficients = np.copy(coefficients)

    for i, j in _coefficient_indices(coefficients):
        coefficients[i, j] /= scale_x ** i * scale_y ** j

    return coefficients


def polyshift2d(coefficients: np.ndarray,
                offset_x: float,
                offset_y: float,
                copy: bool = True) -> np.ndarray:
    """
    Rewrite coefficients of `P(x - offset_x, y - offset_y)` as coefficients of `P(x, y)`.

    Expands the shifted monomials binomially and accumulates the contributions each
    higher-order term makes to the lower-order ones.
    """
    if copy:
        coefficients = np.copy(coefficients)

    idx = _coefficient_indices(coefficients)
    # The originals are needed throughout, but the loop below mutates coefficients
    original = np.copy(coefficients)

    for k, m in idx:
        not_the_same = ~((idx[:, 0] == k) & (idx[:, 1] == m))
        above = (idx[:, 0] >= k) & (idx[:, 1] >= m) & not_the_same

        for i, j in idx[above]:
            b = binom(i, k) * binom(j, m)
            sign = (-1) ** ((i - k) + (j - m))
            offset = offset_x ** (i - k) * offset_y ** (j - m)
            coefficients[k, m] += sign * b * original[i, j] * offset

    return coefficients


def polyfit2d(x: np.ndarray,
              y: np.ndarray,
              z: np.ndarray,
              degree: int | tuple[int, int] = 1,
              *,
              max_degree: int | None = None,
              scale: bool = True) -> np.ndarray:
    """
    Least squares fit of a bivariate polynomial `z = P(x, y)`.

    Parameters
    ----------
    x, y : np.ndarray
        Coordinates of the samples. Flattened; masked entries are dropped.
    z : np.ndarray
        Values to fit.
    degree : int | tuple[int, int]
        Polynomial degree, either shared or as `(degree_x, degree_y)`.
    max_degree : int, optional
        If given, drop every term whose combined degree `i + j` exceeds this. Useful to
        avoid spending coefficients on high cross terms that the data cannot constrain.
    scale : bool
        Whether to normalise the coordinates before fitting. Leave enabled unless the
        inputs are already of order unity.

    Returns
    -------
    np.ndarray
        Coefficients of shape `(degree_x + 1, degree_y + 1)`, where `coeff[i, j]`
        multiplies `x**i * y**j`. Evaluate with
        `numpy.polynomial.polynomial.polyval2d`, or with `polyval2d_safe` below.

    Raises
    ------
    ValueError
        If `degree` is not one or two values, or if there are fewer samples than
        coefficients to solve for.
    """
    x = np.asarray(x).ravel()
    y = np.asarray(y).ravel()
    z = np.asarray(z).ravel()

    keep = ~(np.ma.getmaskarray(x) | np.ma.getmaskarray(y) | np.ma.getmaskarray(z))
    x, y, z = x[keep], y[keep], z[keep]

    if np.isscalar(degree):
        degree = (int(degree), int(degree))
    if len(degree) != 2:
        raise ValueError(f"Only 2D polynomials can be fitted, got degree {degree}")
    degree = (int(degree[0]), int(degree[1]))

    coefficients = np.zeros((degree[0] + 1, degree[1] + 1))
    idx = _coefficient_indices(coefficients)

    if max_degree is not None:
        idx = idx[idx[:, 0] + idx[:, 1] <= int(max_degree)]

    if x.size < len(idx):
        raise ValueError(f"Cannot fit {len(idx)} coefficients to {x.size} samples")

    if scale:
        x, y, norm, offset = _scale(x, y)

    vandermonde = np.polynomial.polynomial.polyvander2d(x, y, degree)
    if max_degree is not None:
        full_idx = _coefficient_indices(coefficients)
        vandermonde = vandermonde[:, full_idx[:, 0] + full_idx[:, 1] <= int(max_degree)]

    solution, *_ = lstsq(vandermonde, z)

    for k, (i, j) in enumerate(idx):
        coefficients[i, j] = solution[k]

    if scale:
        coefficients = polyscale2d(coefficients, *norm, copy=False)
        coefficients = polyshift2d(coefficients, *offset, copy=False)

    return coefficients


def polyval2d_safe(x: np.ndarray, y: np.ndarray, coefficients: np.ndarray) -> np.ndarray:
    """
    Evaluate a bivariate polynomial, broadcasting `x` and `y` against each other.

    `numpy.polynomial.polynomial.polyval2d` requires the two coordinate arrays to have
    the same shape, which is inconvenient when evaluating over a grid. This broadcasts
    them first.
    """
    x, y = np.broadcast_arrays(np.asarray(x, dtype=float), np.asarray(y, dtype=float))
    return polyval2d(x, y, coefficients)
