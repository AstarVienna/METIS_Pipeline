"""
Unit tests for weighted_polyfit.

Each test exercises one property: agreement with np.polyfit, exact recovery
of a polynomial from noiseless data, batch independence, weighting behaviour,
covariance correctness, and handling of singular pixels.
"""
import numpy as np

from pymetis.engine.core.functions.polyfit import weighted_polyfit


def single_pixel(x, y, w, deg):
    """Wrap weighted_polyfit as if it operated on one pixel."""
    x3 = x[:, None, None]
    y3 = y[:, None, None]
    w3 = w[:, None, None]
    p, cov, ok = weighted_polyfit(x3, y3, deg, weights=w3)
    return p[:, 0, 0], cov[:, :, 0, 0], bool(ok[0, 0])


# ---------- tests ----------


class TestWeightedPolyfit:
    def test_matches_numpy_polyfit_unweighted(self):
        """With unit weights, results should match np.polyfit."""
        rng = np.random.default_rng(0)
        x = np.linspace(-2, 2, 20)
        y = 1.0 + 2.0 * x - 0.5 * x**2 + 0.1 * x**3 + rng.normal(0, 0.05, 20)
        w = np.ones_like(x)

        p_ref, cov_ref = np.polyfit(x, y, 3, w=w, cov='unscaled')
        p_ours, cov_ours, ok = single_pixel(x, y, w, 3)

        assert ok
        np.testing.assert_allclose(p_ours, p_ref, rtol=1e-10, atol=1e-14)
        np.testing.assert_allclose(cov_ours, cov_ref, rtol=1e-10, atol=1e-14)


    def test_matches_numpy_polyfit_weighted(self):
        """Non-uniform weights should still match np.polyfit."""
        rng = np.random.default_rng(1)
        x = np.linspace(0, 100, 30)        # offset & rescaled to exercise the scaling
        true_p = np.array([0.001, -0.05, 2.0, 10.0])
        y = np.polyval(true_p, x) + rng.normal(0, 0.5, 30)
        w = rng.uniform(0.5, 2.0, 30)

        p_ref, cov_ref = np.polyfit(x, y, 3, w=w, cov='unscaled')
        p_ours, cov_ours, ok = single_pixel(x, y, w, 3)

        assert ok
        np.testing.assert_allclose(p_ours, p_ref, rtol=1e-8)
        np.testing.assert_allclose(cov_ours, cov_ref, rtol=1e-8)


    def test_exact_recovery_noiseless(self):
        """A polynomial sampled without noise should be recovered exactly."""
        x = np.linspace(-5, 5, 10)
        true_p = np.array([0.3, -1.2, 0.7, 4.0])     # cubic, highest-degree first
        y = np.polyval(true_p, x)
        w = np.ones_like(x)

        p, cov, ok = single_pixel(x, y, w, 3)
        assert ok
        np.testing.assert_allclose(p, true_p, atol=1e-10)


    def test_pixels_are_independent(self):
        """Each pixel should be fit independently — shuffling one mustn't affect others."""
        rng = np.random.default_rng(2)
        n, h, w = 15, 4, 4
        x = rng.uniform(0, 10, (n, h, w))
        y = rng.uniform(0, 10, (n, h, w))
        w = np.ones_like(x)

        p1, _, _ = weighted_polyfit(x, y, 2, weights=w)

        # Replace pixel (0, 0) with totally different data; others must be unchanged.
        x2 = x.copy(); y2 = y.copy()
        x2[:, 0, 0] = rng.uniform(0, 10, n)
        y2[:, 0, 0] = rng.uniform(0, 10, n)
        p2, _, _ = weighted_polyfit(x2, y2, 2, weights=w)

        np.testing.assert_allclose(p1[:, 1:, :], p2[:, 1:, :], rtol=1e-12)
        np.testing.assert_allclose(p1[:, 0, 1:], p2[:, 0, 1:], rtol=1e-12)
        assert not np.allclose(p1[:, 0, 0], p2[:, 0, 0])


    def test_zero_weight_excludes_sample(self):
        """Setting weights=0 on a sample should be equivalent to dropping it entirely."""
        x_full = np.array([1., 2., 3., 4., 5., 6.])
        y_full = np.array([2., 5., 10., 17., 26., 1000.])    # last point is an outlier
        w_full = np.array([1., 1., 1., 1., 1., 0.])          # ignore the outlier

        p_masked, _, ok = single_pixel(x_full, y_full, w_full, 2)
        assert ok

        # Compare to fitting only the first 5 points.
        p_ref = np.polyfit(x_full[:5], y_full[:5], 2)
        np.testing.assert_allclose(p_masked, p_ref, rtol=1e-10, atol=1e-12)


    def test_singular_pixel_flagged(self):
        """A pixel with fewer good samples than coefficients should fail the conditioning check."""
        N, H, W = 10, 2, 2
        rng = np.random.default_rng(3)
        x = rng.uniform(1, 10, (N, H, W))
        y = rng.uniform(1, 10, (N, H, W))
        w = np.ones_like(x)

        # Zero out almost all weights at pixel (0, 0): only 2 samples for a degree-3 fit.
        w[:, 0, 0] = 0
        w[:2, 0, 0] = 1

        _, _, ok = weighted_polyfit(x, y, 3, weights=w)
        assert not ok[0, 0]
        assert ok[0, 1] and ok[1, 0] and ok[1, 1]


    def test_large_x_values_stay_conditioned(self):
        """The rescaling trick should keep fits valid even when x ~ 10^5."""
        rng = np.random.default_rng(4)
        x = np.linspace(1e4, 1e5, 30)
        true_p = np.array([1e-12, -1e-6, 0.5, 100.0])    # tiny leading coeff
        y = np.polyval(true_p, x) + rng.normal(0, 0.01, 30)
        w = np.ones_like(x)

        p_ref, _ = np.polyfit(x, y, 3, w=w, cov='unscaled')
        p_ours, _, ok = single_pixel(x, y, w, 3)

        assert ok
        np.testing.assert_allclose(p_ours, p_ref, rtol=1e-6)


    def test_output_shapes(self):
        """
        Output shapes must be (P, H, W), (P, P, H, W) and (H, W) for the `ok` indicator.
        """
        N, H, W, deg = 8, 5, 7, 2
        x = np.tile(np.linspace(0, 1, N)[:, None, None], (1, H, W))
        y = np.ones((N, H, W))
        w = np.ones((N, H, W))

        p, cov, ok = weighted_polyfit(x, y, deg, weights=w)
        assert p.shape == (deg + 1, H, W)
        assert cov.shape == (deg + 1, deg + 1, H, W)
        assert ok.shape == (H, W)
