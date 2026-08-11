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

Container for the geometry of a single spectral trace.

Adapted from PyReduce (`pyreduce.trace_model`), whose algorithms the METIS DRLD
prescribes for IFU distortion correction (DRLD section "IFU distortion correction",
critical algorithm 5b: "Use the same algorithms as for LSS (PyReduce), but via their
incarnation in the CRIRES pipeline").

Original authors: Nikolai Piskunov, Thomas Marquart, Ansgar Wehrhahn (GPLv3).
"""

from dataclasses import dataclass

import numpy as np


@dataclass
class Trace:
    """
    Geometry of a single spectral trace.

    For the METIS IFU a "trace" is one spatial slice of the image slicer as it lands
    on a detector; for a classical echelle spectrograph it is one spectral order.
    Either way it is a band of illuminated pixels whose mid-line follows a smooth
    function of the dispersion coordinate.

    Attributes
    ----------
    m : int | None
        Trace index, counted from the bottom of the detector upwards. For an echelle
        spectrograph this would be the physical diffraction order; for the IFU it is
        simply the slice number, since slices are numbered according to the optical
        design of the spectrograph.
    pos : np.ndarray
        Coefficients of the trace mid-line `y(x)`, in `np.polyval` order
        (highest power first). Shape `(degree + 1,)`.
    column_range : tuple[int, int]
        Valid dispersion-coordinate range `[start, end)` for this trace.
    bottom, top : np.ndarray | None
        Coefficients of the lower and upper edge of the trace, `y(x)`, in the same
        `np.polyval` order and shape as `pos`. Measured from the image where the
        cross-dispersion profile falls to half its height above the local background,
        so they describe the illuminated extent rather than the spacing to the
        neighbours. `None` when the edges could not be measured.
    height : float | None
        Extraction aperture height in pixels. Derived from `bottom`/`top` when those
        are present, otherwise from the distance to the neighbouring traces. `None` if
        it could not be determined. A single number cannot express an extent that
        varies along the dispersion direction, so prefer the edges where available.
    residual : float | None
        RMS deviation, in pixels, between the fitted mid-line and the pixels it was
        fitted to. Feeds the `QC IFU DISTORT RMS` quality control parameter.
    slit : np.ndarray | None
        Slit tilt coefficients, shape `(degree_y + 1, degree_x + 1)`, describing
        `x_offset = sum_k c_k(x) * y**k` where each `c_k` is itself a polynomial in
        the dispersion coordinate. Not populated yet -- reserved for slit tilt
        determination, which needs a line-rich calibration frame.
    slitdelta : np.ndarray | None
        Per-row residual slit correction beyond the polynomial fit, shape
        `(height_pixels,)`. Reserved, as for `slit`.
    """

    m: int | None
    pos: np.ndarray
    column_range: tuple[int, int]
    bottom: np.ndarray | None = None
    top: np.ndarray | None = None
    height: float | None = None
    residual: float | None = None
    slit: np.ndarray | None = None
    slitdelta: np.ndarray | None = None

    @property
    def degree(self) -> int:
        """Polynomial degree of the trace mid-line."""
        return len(self.pos) - 1

    @property
    def has_edges(self) -> bool:
        """Whether measured edge polynomials are available for this trace."""
        return self.bottom is not None and self.top is not None

    def bottom_at_x(self, x: np.ndarray | float) -> np.ndarray | None:
        """
        Evaluate the lower edge of the trace at the given dispersion coordinates.

        Returns `None` if the edges were never measured, in which case the caller must
        fall back on `height` about `y_at_x`.
        """
        return None if self.bottom is None else np.polyval(self.bottom, x)

    def top_at_x(self, x: np.ndarray | float) -> np.ndarray | None:
        """
        Evaluate the upper edge of the trace at the given dispersion coordinates.

        Returns `None` if the edges were never measured.
        """
        return None if self.top is None else np.polyval(self.top, x)

    def height_at_x(self, x: np.ndarray | float) -> np.ndarray | float | None:
        """
        Cross-dispersion extent of the trace at the given dispersion coordinates.

        Uses the measured edges where available, so the extent may vary along the
        detector; falls back on the constant `height` otherwise.
        """
        if self.has_edges:
            return np.polyval(self.top, x) - np.polyval(self.bottom, x)
        return self.height

    def y_at_x(self, x: np.ndarray | float) -> np.ndarray:
        """
        Evaluate the trace mid-line at the given dispersion coordinates.

        Parameters
        ----------
        x : np.ndarray | float
            Column position(s) to evaluate at.

        Returns
        -------
        np.ndarray
            Cross-dispersion position of the trace centre at each `x`.
        """
        return np.polyval(self.pos, x)

    def slit_at_x(self, x: np.ndarray | float) -> np.ndarray | None:
        """
        Evaluate the slit tilt coefficients at the given dispersion coordinates.

        Parameters
        ----------
        x : np.ndarray | float
            Column position(s) to evaluate at.

        Returns
        -------
        np.ndarray | None
            Coefficients of `x_offset = c0 + c1 * y + c2 * y**2 + ...`, of shape
            `(degree_y + 1,)` for scalar `x` or `(degree_y + 1, len(x))` for array
            `x`. `None` if no slit tilt has been determined.
        """
        if self.slit is None:
            return None

        # slit[k] holds the coefficients of c_k as a polynomial in x
        return np.array([np.polyval(c, x) for c in self.slit])
