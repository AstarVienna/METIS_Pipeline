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
import pytest
from pytest import approx

from pymetis.drl.rsrf import create_blackbody_image, extract_1d_spectra
from pymetis.drl.trace_model import Trace


class TestBlackBodyImage:
    def test_blackbody_image(self):
        x, y = np.meshgrid(np.arange(512), np.arange(512))

        # wavecal image with spectral dimension along x
        wavecal_data = 3.55 + (x / 512.0 * 0.37)
        wavecal_img = cpl.core.Image(wavecal_data)

        bb_temp = 800   # K
        bb_img = create_blackbody_image(wavecal_img, bb_temp)

        assert bb_img.get_median() == approx(3.2575, rel=1e-3)
        assert bb_img.get_min() == approx(3.21761, rel=1e-3)
        assert bb_img.get_max() == approx(3.265, rel=1e-3)

    def test_pixels_without_wavelength_are_rejected(self):
        wavecal_data = np.full((16, 16), 3.7)
        wavecal_data[:, :4] = 0.0
        bb_img = create_blackbody_image(cpl.core.Image(wavecal_data), 800)

        assert bb_img.count_rejected() == 16 * 4
        assert bb_img.get_min() > 0

    def test_empty_wavelength_map_is_rejected(self):
        with pytest.raises(ValueError):
            create_blackbody_image(cpl.core.Image(np.zeros((8, 8))), 800)


class TestExtract1dSpectra:
    def test_extract_traces(self):
        # build a dummy trace list: straight lines y = 0.01 x + (i + 1) * 80
        trace_list = [
            Trace(m=i + 1, pos=np.array([1.0e-2, (i + 1) * 80.0]), column_range=(0, 512))
            for i in range(5)
        ]

        # create a gradient image
        x, y = np.meshgrid(np.arange(512), np.arange(512))
        data = 30.0 + (x / 512.0 * 100) + (y / 512.0 * 200)
        img = cpl.core.Image(data)

        # extract 1d traces from the image
        traces_1d = extract_1d_spectra(img, trace_list, trace_width=10)

        # something to compare against
        med = [np.median(traces_1d[i]) for i in np.arange(5)]

        assert med == approx([111.73828125, 142.98828125, 174.23828125,
                              205.48828125, 236.73828125])
