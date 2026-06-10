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
"""


import numpy as np
from cpl.core import (Image as CplImage,
                      Type as CplType)


def weighted_polyfit(x: np.ndarray,
                     y: np.ndarray,
                     deg: int,
                     *,
                     weights: np.ndarray = None) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Per-pixel weighted polynomial fit, fully vectorized.
    Returns coeffs (highest degree first, like np.polyfit), unscaled cov, ok mask.

    Internally rescales x per pixel to keep the normal-equations matrix
    well-conditioned (matching np.polyfit's behaviour).
    """
    assert x.shape == y.shape, \
        f"Shapes of x and y must match (got {x.shape} and {y.shape})"

    if weights is None:
        weights = np.ones_like(x)
    assert weights.shape == x.shape, \
        f"Weights must have the same shape as data if defined"

    n, h, w = x.shape
    dtype = np.result_type(x, y, weights, np.float64)

    # --- per-pixel scale: divide x by max(|x|) over samples with nonzero weight ---
    mask = (weights != 0)
    x_abs = np.where(mask, np.abs(x), 0.0)
    scale = x_abs.max(axis=0)  # (H, W)
    scale = np.where(scale > 0, scale, 1.0)
    xs = x / scale[None, :, :]  # (N, H, W)

    xp = np.empty((deg + 1, n, h, w), dtype=dtype)
    xp[0] = 1.0
    for k in range(1, deg + 1):
        xp[k] = xp[k - 1] * xs

    w2 = weights * weights

    A = np.empty((deg + 1, deg + 1, h, w), dtype=dtype)
    for i in range(deg + 1):
        for j in range(i, deg + 1):
            if i + j < deg + 1:
                s = (w2 * xp[i + j]).sum(axis=0)
            else:
                s = (w2 * xp[i] * xp[j]).sum(axis=0)
            A[i, j] = s
            A[j, i] = s

    b = (w2 * xp * y).sum(axis=1)  # (P, H, W)

    A = np.moveaxis(A, (0, 1), (-2, -1))  # (H, W, P, P)
    b = np.moveaxis(b, 0, -1)  # (H, W, P)

    ok = np.linalg.cond(A) < 1.0 / np.finfo(dtype).eps

    coeffs_s = np.zeros_like(b)
    cov_s = np.zeros_like(A)

    if ok.any():
        Ag = A[ok]  # (K, P, P)
        bg = b[ok][..., None]  # (K, P, 1) — solve needs (m,n)
        coeffs_s[ok] = np.linalg.solve(Ag, bg)[..., 0]
        cov_s[ok] = np.linalg.inv(Ag)

    # Transform back from scaled-x basis: c[k] = c_s[k] / scale**k
    powers = scale[..., None] ** np.arange(deg + 1)  # (H, W, P)
    coeffs = coeffs_s / powers  # (H, W, P)
    cov = cov_s / (powers[..., :, None] * powers[..., None, :])  # (H, W, P, P)

    coeffs_poly = np.moveaxis(coeffs, -1, 0)[::-1]  # (P, H, W)
    cov_poly = np.moveaxis(cov, (-2, -1), (0, 1))[::-1, ::-1]  # (P, P, H, W)
    return coeffs_poly, cov_poly, ok