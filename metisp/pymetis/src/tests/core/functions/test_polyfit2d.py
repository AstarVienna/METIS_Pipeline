"""
Unit tests for polyfit2d.

Each test exercises one property: exact recovery of a known bivariate polynomial,
robustness at detector-scale coordinates, the max_degree restriction, broadcasting in
the evaluator, and the guards against an underdetermined or malformed request.
"""
import numpy as np
import pytest

from pymetis.engine.core.functions.polyfit2d import (polyfit2d, polyscale2d,
                                                     polyshift2d, polyval2d_safe)


def sample_grid(nx=40, ny=12, seed=0):
    """Scattered samples over a detector-sized domain."""
    rng = np.random.default_rng(seed)
    x = rng.uniform(0, 2048, nx * ny)
    y = rng.uniform(-60, 60, nx * ny)
    return x, y


class TestPolyfit2d:
    def test_recovers_known_polynomial(self):
        """A noiseless polynomial is recovered to machine precision."""
        x, y = sample_grid()
        true = np.array([[3.55, 1.1e-5], [1.3e-5, -2.0e-9], [-5.0e-11, 3.0e-14]])

        coefficients = polyfit2d(x, y, polyval2d_safe(x, y, true), degree=(2, 1))

        np.testing.assert_allclose(coefficients, true, rtol=1e-6, atol=1e-18)

    def test_recovers_values_at_pixel_scale(self):
        """
        Fitted values match the data even with coordinates in the thousands.

        This is what the internal rescaling is for: a fourth power of 2048 overflows
        the useful precision of an unscaled least squares solve.
        """
        x, y = sample_grid(seed=1)
        true = np.zeros((4, 2))
        true[0, 0], true[1, 0], true[2, 0], true[3, 0] = 3.5, 1e-5, -2e-9, 4e-13
        true[0, 1], true[1, 1] = -1.4e-7, 2e-11
        z = polyval2d_safe(x, y, true)

        coefficients = polyfit2d(x, y, z, degree=(3, 1))

        np.testing.assert_allclose(polyval2d_safe(x, y, coefficients), z,
                                   rtol=0, atol=1e-12)

    def test_scaling_can_be_disabled(self):
        """With small coordinates the unscaled path agrees with the scaled one."""
        rng = np.random.default_rng(2)
        x, y = rng.uniform(-1, 1, 200), rng.uniform(-1, 1, 200)
        true = np.array([[1.0, 0.5], [-0.25, 0.125]])
        z = polyval2d_safe(x, y, true)

        scaled = polyfit2d(x, y, z, degree=(1, 1), scale=True)
        unscaled = polyfit2d(x, y, z, degree=(1, 1), scale=False)

        np.testing.assert_allclose(scaled, unscaled, rtol=1e-8, atol=1e-12)

    def test_scalar_degree_applies_to_both_axes(self):
        x, y = sample_grid(seed=3)
        z = polyval2d_safe(x, y, np.eye(3) * 1e-6)

        assert polyfit2d(x, y, z, degree=2).shape == (3, 3)

    def test_max_degree_drops_high_cross_terms(self):
        """Terms whose combined degree exceeds max_degree are left at zero."""
        x, y = sample_grid(seed=4)
        z = polyval2d_safe(x, y, np.array([[3.5, 1e-6], [1e-5, 0.0]]))

        coefficients = polyfit2d(x, y, z, degree=(2, 2), max_degree=1)

        assert coefficients.shape == (3, 3)
        assert coefficients[2, 2] == 0.0
        assert coefficients[1, 1] == 0.0
        np.testing.assert_allclose(polyval2d_safe(x, y, coefficients), z,
                                   rtol=0, atol=1e-9)

    def test_masked_samples_are_dropped(self):
        x, y = sample_grid(seed=5)
        true = np.array([[3.5, 1e-6], [1e-5, 0.0]])
        z = np.ma.masked_array(polyval2d_safe(x, y, true), mask=False)
        z.mask[:50] = True

        coefficients = polyfit2d(x, y, z, degree=(1, 1))

        np.testing.assert_allclose(coefficients, true, rtol=1e-6, atol=1e-15)

    def test_rejects_underdetermined_fit(self):
        with pytest.raises(ValueError, match='coefficients'):
            polyfit2d(np.arange(3.0), np.arange(3.0), np.arange(3.0), degree=(2, 2))

    def test_rejects_malformed_degree(self):
        x, y = sample_grid(seed=6)
        with pytest.raises(ValueError, match='2D polynomials'):
            polyfit2d(x, y, np.zeros_like(x), degree=(1, 2, 3))

    def test_degenerate_coordinate_does_not_divide_by_zero(self):
        """All samples at one y is fine as long as the y degree is zero."""
        x = np.linspace(0, 2048, 50)
        y = np.zeros_like(x)
        z = 3.5 + 1e-5 * x

        coefficients = polyfit2d(x, y, z, degree=(1, 0))

        np.testing.assert_allclose(polyval2d_safe(x, y, coefficients), z, atol=1e-12)


class TestPolyval2dSafe:
    def test_broadcasts_mismatched_shapes(self):
        """A row and a column vector evaluate over their outer grid."""
        coefficients = np.array([[0.0, 1.0], [1.0, 0.0]])

        result = polyval2d_safe(np.arange(5)[None, :], np.arange(3)[:, None],
                                coefficients)

        assert result.shape == (3, 5)
        # coeff encodes x + y
        np.testing.assert_allclose(result,
                                   np.arange(5)[None, :] + np.arange(3)[:, None])


class TestCoefficientTransforms:
    def test_scale_then_evaluate_matches_scaled_input(self):
        """polyscale2d rewrites P(x/s, y/t) as a polynomial in x and y."""
        rng = np.random.default_rng(7)
        coefficients = rng.normal(size=(3, 2))
        x, y = rng.uniform(-2, 2, 30), rng.uniform(-2, 2, 30)

        rescaled = polyscale2d(coefficients, 3.0, 5.0)

        np.testing.assert_allclose(polyval2d_safe(x, y, rescaled),
                                   polyval2d_safe(x / 3.0, y / 5.0, coefficients),
                                   rtol=1e-10)

    def test_shift_then_evaluate_matches_shifted_input(self):
        """polyshift2d rewrites P(x - a, y - b) as a polynomial in x and y."""
        rng = np.random.default_rng(8)
        coefficients = rng.normal(size=(3, 3))
        x, y = rng.uniform(-2, 2, 30), rng.uniform(-2, 2, 30)

        shifted = polyshift2d(coefficients, 1.5, -0.5)

        np.testing.assert_allclose(polyval2d_safe(x, y, shifted),
                                   polyval2d_safe(x - 1.5, y + 0.5, coefficients),
                                   rtol=1e-10)

    def test_copy_leaves_the_original_untouched(self):
        coefficients = np.ones((2, 2))

        polyscale2d(coefficients, 2.0, 2.0, copy=True)
        polyshift2d(coefficients, 1.0, 1.0, copy=True)

        np.testing.assert_array_equal(coefficients, np.ones((2, 2)))
