"""
Unit tests for the ported PyReduce rectification.

The tests build small synthetic frames whose answer is known analytically, rather than
using detector-sized data: straightening a trace is a geometric operation, so a frame
with a deliberately curved feature is enough to show that the curvature is removed.
Each test exercises one property: that a curved trace comes out straight, that the strip
follows the mid-line, that off-detector rows are marked rather than wrapped, that the
resampling lands on the requested grid, and that the cube stacks slices without
extrapolating beyond each slice's own coverage.
"""
import numpy as np
import pytest

from pymetis.drl.rectify import (build_cube, default_height, linear_wavelength_grid,
                                 rectify_image, rectify_trace,
                                 resample_to_wavelength_grid, strip_index)
from pymetis.drl.trace_model import Trace

NROW, NCOL = 200, 120


def curved_trace(offset: float = 100.0, curvature: float = 2.0e-3) -> Trace:
    """A quadratic mid-line spanning the full width, as `metis_ifu_distortion` fits."""
    return Trace(m=0,
                 pos=np.array([curvature, 0.0, offset]),
                 column_range=(0, NCOL))


def frame_with_ridge(trace: Trace, sigma: float = 3.0) -> np.ndarray:
    """A frame holding a single Gaussian ridge centred on `trace`."""
    columns = np.arange(NCOL)
    rows = np.arange(NROW)[:, None]
    centre = trace.y_at_x(columns)
    return 1000.0 * np.exp(-0.5 * ((rows - centre[None, :]) / sigma) ** 2)


class TestStripIndex:
    def test_shape_and_placement(self) -> None:
        centres = np.full(NCOL, 50.0)
        rows, columns = strip_index(centres, 7, 10, 30)

        assert rows.shape == columns.shape == (7, 20)
        # the mid-line lands on the centre row
        assert np.all(rows[3] == 50)
        assert np.all(columns[0] == np.arange(10, 30))

    def test_follows_a_varying_centre(self) -> None:
        centres = np.arange(NCOL, dtype=float)
        rows, _ = strip_index(centres, 3, 0, 5)
        assert np.all(rows[1] == np.arange(5))

    def test_rounds_rather_than_truncates(self) -> None:
        # 9.6 must land on row 10, not row 9
        rows, _ = strip_index(np.full(NCOL, 9.6), 1, 0, 1)
        assert rows[0, 0] == 10

    @pytest.mark.parametrize("height, first, last", [(0, 0, 10), (-1, 0, 10), (3, 5, 5)])
    def test_rejects_degenerate_geometry(self, height, first, last) -> None:
        with pytest.raises(ValueError):
            strip_index(np.zeros(NCOL), height, first, last)


class TestRectifyTrace:
    def test_curved_ridge_comes_out_straight(self) -> None:
        trace = curved_trace()
        strip = rectify_trace(frame_with_ridge(trace), trace, height=21)

        # The ridge must sit on the same row in every column once straightened; on the
        # raw frame it moves by curvature * NCOL**2 pixels.
        peak_rows = np.argmax(strip, axis=0)
        assert peak_rows.min() == peak_rows.max() == 10
        assert trace.y_at_x(NCOL - 1) - trace.y_at_x(0) > 20, "trace was not curved"

    def test_honours_the_column_range(self) -> None:
        trace = Trace(m=0, pos=np.array([0.0, 0.0, 60.0]), column_range=(20, 90))
        strip = rectify_trace(frame_with_ridge(trace), trace, height=5)
        assert strip.shape == (5, 70)

    def test_off_detector_rows_are_nan_not_wrapped(self) -> None:
        # A trace at row 2 with a tall strip runs off the bottom of the frame
        trace = Trace(m=0, pos=np.array([0.0, 0.0, 2.0]), column_range=(0, NCOL))
        strip = rectify_trace(frame_with_ridge(trace), trace, height=11)

        assert np.isnan(strip[0]).all(), "rows below the detector must be NaN"
        assert not np.isnan(strip[-1]).any(), "rows inside the detector must be kept"


class TestDefaultHeight:
    def test_derived_from_spacing_and_odd(self) -> None:
        traces = [Trace(m=k, pos=np.array([0.0, 0.0, 10.0 + 20.0 * k]),
                        column_range=(0, NCOL)) for k in range(5)]
        height = default_height(traces)
        assert height == 21, "20 px spacing, rounded up to an odd height"

    def test_single_trace_falls_back(self) -> None:
        assert default_height([curved_trace()], fallback=8) == 9


class TestRectifyImage:
    def test_one_strip_per_trace(self) -> None:
        traces = [Trace(m=k, pos=np.array([0.0, 0.0, 30.0 + 40.0 * k]),
                        column_range=(0, NCOL)) for k in range(3)]
        frame = sum(frame_with_ridge(t) for t in traces)
        strips = rectify_image(frame, traces, height=9)

        assert len(strips) == 3
        for strip in strips:
            assert strip.shape == (9, NCOL)
            assert np.argmax(strip[:, 0]) == 4

    def test_no_traces_yields_nothing(self) -> None:
        assert rectify_image(np.zeros((NROW, NCOL)), [], height=5) == []


class TestLinearWavelengthGrid:
    def test_spans_the_valid_wavelengths(self) -> None:
        grid = linear_wavelength_grid(np.array([3.5, 0.0, 3.7, np.nan]), samples=5)
        assert grid[0] == pytest.approx(3.5)
        assert grid[-1] == pytest.approx(3.7)
        assert len(grid) == 5
        assert np.all(np.diff(grid) > 0)

    def test_rejects_an_empty_map(self) -> None:
        with pytest.raises(ValueError, match="No valid wavelength"):
            linear_wavelength_grid(np.zeros(10))

    def test_rejects_a_single_wavelength(self) -> None:
        with pytest.raises(ValueError, match="identical"):
            linear_wavelength_grid(np.full(10, 3.5))


class TestResampleToWavelengthGrid:
    def test_reproduces_a_linear_ramp(self) -> None:
        waves = np.linspace(3.0, 4.0, 50)
        strip = np.tile(waves, (3, 1))          # value == wavelength
        grid = np.linspace(3.0, 4.0, 25)

        resampled = resample_to_wavelength_grid(strip, waves, grid)
        assert resampled.shape == (3, 25)
        np.testing.assert_allclose(resampled[0], grid, rtol=1e-10)

    def test_does_not_extrapolate(self) -> None:
        waves = np.linspace(3.4, 3.6, 20)
        strip = np.ones((2, 20))
        grid = np.linspace(3.0, 4.0, 11)

        resampled = resample_to_wavelength_grid(strip, waves, grid)
        assert np.isnan(resampled[:, 0]).all(), "below the strip's coverage"
        assert np.isnan(resampled[:, -1]).all(), "above the strip's coverage"
        assert not np.isnan(resampled[:, 5]).any(), "inside the coverage"

    def test_width_must_match(self) -> None:
        with pytest.raises(ValueError, match="columns wide"):
            resample_to_wavelength_grid(np.ones((2, 10)), np.linspace(3, 4, 9),
                                        np.linspace(3, 4, 5))


class TestBuildCube:
    @staticmethod
    def slices(n: int = 4, width: int = 30):
        strips = [np.full((5, width), float(k)) for k in range(n)]
        waves = [np.linspace(3.4 + 0.01 * k, 3.6 + 0.01 * k, width) for k in range(n)]
        return strips, waves

    def test_shape_and_axis(self) -> None:
        strips, waves = self.slices()
        grid, cube = build_cube(strips, waves)

        assert cube.shape == (4, 5, len(grid))
        assert np.all(np.diff(grid) > 0)
        assert grid[0] == pytest.approx(3.4)
        assert grid[-1] == pytest.approx(3.6 + 0.03)

    def test_each_slice_keeps_its_own_value(self) -> None:
        strips, waves = self.slices()
        grid, cube = build_cube(strips, waves)

        for k in range(4):
            covered = cube[k][np.isfinite(cube[k])]
            assert covered.size > 0
            np.testing.assert_allclose(covered, float(k), atol=1e-9)

    def test_uncovered_wavelengths_are_nan(self) -> None:
        strips, waves = self.slices()
        _, cube = build_cube(strips, waves)
        # slice 0 stops short of the reddest wavelengths, which slice 3 supplies
        assert np.isnan(cube[0, :, -1]).all()
        assert not np.isnan(cube[3, :, -1]).any()

    def test_rejects_mismatched_input(self) -> None:
        strips, waves = self.slices()
        with pytest.raises(ValueError, match="No strips"):
            build_cube([], [])
        with pytest.raises(ValueError, match="wavelength arrays"):
            build_cube(strips, waves[:2])
        with pytest.raises(ValueError, match="disagree in height"):
            build_cube([np.ones((5, 10)), np.ones((7, 10))],
                       [np.linspace(3, 4, 10)] * 2)
