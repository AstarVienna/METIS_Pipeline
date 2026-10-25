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
import cpl
from pytest import approx

from pymetis.drl.pinhole import locate_pinhole


def gaussian_image(xcen: float, ycen: float, sigma: float, size: int = 512) -> cpl.core.Image:
    x, y = np.meshgrid(np.arange(size), np.arange(size))
    img = np.exp(-((x - xcen) ** 2 + (y - ycen) ** 2) / (2 * sigma ** 2))
    return cpl.core.Image(img.astype(np.float32))


class TestLocatePinhole:
    def test_centroid_and_fwhm_of_a_gaussian(self):
        xcen, ycen, sigma = 234.5, 378.2, 2.5
        result = locate_pinhole(gaussian_image(xcen, ycen, sigma), 15)
        # CPL centroids are 1-based
        assert result['xcen'] == approx(xcen + 1)
        assert result['ycen'] == approx(ycen + 1)
        # fwhm measurement is not very good, hence large tolerance
        assert result['fwhm_x'] == approx(2.355 * sigma, abs=2)
        assert result['fwhm_y'] == approx(2.355 * sigma, abs=2)

    def test_result_has_all_keys(self):
        result = locate_pinhole(gaussian_image(100.0, 200.0, 2.0), 10)
        assert set(result) == {'xcen', 'ycen', 'xcenstd', 'ycenstd', 'fwhm_x', 'fwhm_y', 'snr'}
        assert result['snr'] > 0

    def test_window_is_clipped_at_the_image_edge(self):
        # pinhole closer to the corner than the half-window: must not raise
        result = locate_pinhole(gaussian_image(3.0, 4.0, 1.5, size=64), 15)
        assert result['xcen'] == approx(4.0, abs=0.5)
        assert result['ycen'] == approx(5.0, abs=0.5)
