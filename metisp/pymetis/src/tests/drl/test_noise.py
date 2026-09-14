"""
Numerical tests of `pymetis.drl.noise`: the noise model and the outlier detection.
"""
import numpy as np
from cpl.core import Image as CplImage
from hdrl.core import Image as HdrlImage

from pymetis.drl.noise import calculate_outliers, estimate_noise, estimate_noise_list


class TestEstimateNoise:
    def test_error_is_shot_noise_plus_read_noise_in_quadrature(self):
        data = np.array([[0.0, 9.0], [16.0, 100.0]])
        estimated = estimate_noise(CplImage(data), read_noise=3.0)
        assert isinstance(estimated, HdrlImage)
        np.testing.assert_allclose(estimated.image.as_array(), data)
        np.testing.assert_allclose(estimated.error.as_array(), np.sqrt(data + 9.0))

    def test_list_wraps_every_frame(self):
        from cpl.core import ImageList as CplImageList
        frames = CplImageList([CplImage(np.full((3, 3), float(v))) for v in (1, 4, 9)])
        estimated = estimate_noise_list(frames, read_noise=0.0)
        assert len(estimated) == 3
        np.testing.assert_allclose(estimated[2].error.as_array(), np.full((3, 3), 3.0))


class TestCalculateOutliers:
    def test_planted_hot_and_cold_pixels_are_found_on_the_right_side(self):
        rng = np.random.default_rng(0)
        data = rng.normal(1000.0, 1.0, size=(64, 64))
        data[10, 20] = 50000.0    # hot
        data[30, 40] = 0.0        # cold (kept non-negative: the error is sqrt(data))
        image = estimate_noise(CplImage(data), read_noise=0.0)

        mask_hot, mask_cold = calculate_outliers(image, kappa_low=5.0, kappa_high=5.0)
        hot, cold = np.asarray(mask_hot).astype(bool), np.asarray(mask_cold).astype(bool)

        assert hot[10, 20] and not cold[10, 20]
        assert cold[30, 40] and not hot[30, 40]
        # Gaussian noise at 5 sigma yields no false positives on 4096 pixels -- and in
        # particular none along the border, which `Border.NOP` used to flag wholesale
        assert hot.sum() == 1 and cold.sum() == 1

    def test_pure_noise_yields_no_outliers(self):
        rng = np.random.default_rng(1)
        image = estimate_noise(CplImage(rng.normal(1000.0, 1.0, size=(64, 64))), read_noise=0.0)

        mask_hot, mask_cold = calculate_outliers(image, kappa_low=5.0, kappa_high=5.0)

        assert np.asarray(mask_hot).sum() == 0
        assert np.asarray(mask_cold).sum() == 0
