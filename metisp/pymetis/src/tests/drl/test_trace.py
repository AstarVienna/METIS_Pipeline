"""
Unit tests for the ported PyReduce order tracing.

The METIS IFU simulated data available for testing is a set of 32x32 pixel stubs, far
too small to contain a trace, so these tests build synthetic full-size detector frames
instead. Each test exercises one property: recovery of known trace polynomials,
recovery of the valid column range, cluster merging across a gap, graceful degradation
on frames with nothing to find, FWHM measurement, and the round trip through the
`IFU_DISTORTION_TABLE` layout that `metis_ifu_rsrf` and `metis_ifu_wavecal` consume.
"""
import numpy as np
import pytest

from pymetis.drl.trace import measure_trace_fwhm, trace, traces_from_table, traces_to_table
from pymetis.drl.trace_model import Trace
from pymetis.instruments.metis.dataitems.distortion import IfuDistortionTable

# Detector size and slice layout of the METIS IFU: a 2048x2048 HAWAII2RG carrying
# 14 of the 28 spatial slices, spaced as in the simulated data.
NROW = NCOL = 2048
N_SLICES = 14
FIRST_SLICE_Y = 193.0
SLICE_SPACING = 127.0
SLICE_SIGMA = 18.0

# Tracing parameters tuned for the METIS IFU in PyReduce, and used as the recipe defaults
TRACE_PARAMETERS = dict(
    degree=2,
    min_cluster=1000,
    filter_y=200,
    noise=120,
    border_width=6,
    auto_merge_threshold=0.9,
    merge_min_threshold=0.01,
)


def slice_coefficients(k: int) -> np.ndarray:
    """Quadratic mid-line of slice `k`, in `np.polyval` order."""
    return np.array([-2.0e-6, 3.0e-3, FIRST_SLICE_Y + k * SLICE_SPACING])


def synthetic_frame(n_slices: int = N_SLICES,
                    noise_sigma: float = 8.0,
                    background: float = 80.0,
                    seed: int = 42,
                    gap: tuple[int, int] | None = None) -> tuple[np.ndarray, list]:
    """
    Build a synthetic IFU detector frame with Gaussian-profile slices.

    Parameters
    ----------
    n_slices : int
        Number of slices to draw.
    noise_sigma, background : float
        Read noise and background level added to the frame.
    seed : int
        Seed for the noise, so tests are deterministic.
    gap : tuple[int, int], optional
        Column range to blank out across every slice, simulating a dead column block
        that splits each slice into two clusters.

    Returns
    -------
    tuple[np.ndarray, list]
        The frame, and the true mid-line coefficients of each slice.
    """
    rng = np.random.default_rng(seed)
    columns = np.arange(NCOL)
    rows = np.arange(NROW)[:, None]

    frame = np.zeros((NROW, NCOL))
    truth = []
    for k in range(n_slices):
        coefficients = slice_coefficients(k)
        truth.append(coefficients)
        centre = np.polyval(coefficients, columns)
        frame += 3000.0 * np.exp(-0.5 * ((rows - centre[None, :]) / SLICE_SIGMA) ** 2)

    if gap is not None:
        frame[:, gap[0]:gap[1]] = 0.0

    frame += rng.normal(background, noise_sigma, frame.shape)
    return frame, truth


class TestTrace:
    def test_finds_every_slice(self):
        """All slices are detected, numbered from the bottom of the detector up."""
        frame, truth = synthetic_frame()

        traces = trace(frame, **TRACE_PARAMETERS)

        assert len(traces) == len(truth)
        assert [t.m for t in traces] == list(range(len(truth)))
        # m must increase with position on the detector
        centres = [t.y_at_x(NCOL // 2) for t in traces]
        assert centres == sorted(centres)

    def test_recovers_trace_polynomials(self):
        """
        The fitted mid-lines match the true ones to well under a pixel.

        The outermost slices are excluded: the background estimate is one-sided at the
        detector edges, which biases the threshold and hence the cluster centroid
        there by a pixel or two. That is a property of the algorithm, not of the port.
        """
        frame, truth = synthetic_frame()

        traces = trace(frame, **TRACE_PARAMETERS)

        assert len(traces) == len(truth)
        columns = np.arange(300, 1800)
        for t, coefficients in list(zip(traces, truth))[1:-1]:
            np.testing.assert_allclose(
                t.y_at_x(columns), np.polyval(coefficients, columns), atol=0.5,
            )

    def test_reports_subpixel_residuals(self):
        """The fit residual measures mid-line accuracy, not slice thickness."""
        frame, _ = synthetic_frame()

        traces = trace(frame, **TRACE_PARAMETERS)

        assert traces
        for t in traces:
            assert t.residual is not None
            # Slice thickness is ~2.35 * 18 px; a residual near that would mean the
            # scatter of individual pixels was being reported instead of the mid-line.
            assert 0.0 < t.residual < 1.0

    def test_recovers_column_range_and_height(self):
        """The valid column range respects the border, and heights match the spacing."""
        frame, _ = synthetic_frame()

        traces = trace(frame, **TRACE_PARAMETERS)

        assert traces
        for t in traces:
            start, end = t.column_range
            assert start >= TRACE_PARAMETERS['border_width']
            assert end <= NCOL - TRACE_PARAMETERS['border_width']
            assert end - start > 0.9 * NCOL

        # Interior traces have a neighbour on both sides, so height is the spacing
        for t in traces[1:-1]:
            assert t.height == pytest.approx(SLICE_SPACING, abs=3.0)

    def test_degree_sets_number_of_coefficients(self):
        """The requested polynomial degree determines the coefficient count."""
        frame, _ = synthetic_frame()

        for degree in (2, 3, 4):
            traces = trace(frame, **{**TRACE_PARAMETERS, 'degree': degree})
            assert traces
            assert all(len(t.pos) == degree + 1 for t in traces)

    def test_merges_clusters_split_by_dead_columns(self):
        """A block of dead columns must not double the number of detected traces."""
        frame, truth = synthetic_frame(gap=(1000, 1060))

        traces = trace(frame, **TRACE_PARAMETERS)

        assert len(traces) == len(truth)

    def test_merging_can_be_disabled(self):
        """An auto_merge_threshold of 1 skips merging, leaving the split clusters."""
        frame, truth = synthetic_frame(gap=(1000, 1060))

        traces = trace(frame, **{**TRACE_PARAMETERS, 'auto_merge_threshold': 1.0})

        assert len(traces) > len(truth)


class TestTraceDegradation:
    """
    Frames with nothing to find must return no traces rather than raising.

    The recipe test data consists of 32x32 stubs, so this path runs on every
    invocation of `metis_ifu_distortion` against the current simulated data.
    """

    def test_blank_frame_yields_no_traces(self):
        assert trace(np.zeros((NROW, NCOL)), **TRACE_PARAMETERS) == []

    def test_pure_noise_yields_no_traces(self):
        rng = np.random.default_rng(0)
        frame = rng.normal(100, 10, (NROW, NCOL))

        assert trace(frame, **TRACE_PARAMETERS) == []

    def test_undersized_frame_yields_no_traces(self):
        """A 32x32 stub has fewer pixels in total than min_cluster."""
        rng = np.random.default_rng(0)

        assert trace(rng.normal(100, 10, (32, 32)), **TRACE_PARAMETERS) == []

    def test_estimation_on_blank_frame_yields_no_traces(self):
        """With filter_y unset it must be estimated, which a blank frame cannot do."""
        parameters = {k: v for k, v in TRACE_PARAMETERS.items() if k != 'filter_y'}

        assert trace(np.zeros((NROW, NCOL)), **parameters) == []

    def test_rejects_invalid_parameters(self):
        frame, _ = synthetic_frame(n_slices=2)

        with pytest.raises(ValueError):
            trace(frame, **{**TRACE_PARAMETERS, 'filter_y': 0})
        with pytest.raises(ValueError):
            trace(frame, **{**TRACE_PARAMETERS, 'border_width': -1})
        with pytest.raises(ValueError):
            trace(frame, **{**TRACE_PARAMETERS, 'filter_type': 'nonsense'})


class TestMeasureTraceFwhm:
    def test_measures_gaussian_fwhm(self):
        """The measured FWHM matches the Gaussian profile the slices were drawn with."""
        frame, _ = synthetic_frame()

        traces = trace(frame, **TRACE_PARAMETERS)
        fwhm = measure_trace_fwhm(frame, traces)

        assert fwhm == pytest.approx(2.3548 * SLICE_SIGMA, rel=0.05)

    def test_returns_none_without_traces(self):
        frame, _ = synthetic_frame()

        assert measure_trace_fwhm(frame, []) is None


class TestDistortionTableRoundTrip:
    """
    The serialised table must be readable by `IfuDistortionTable.read()`.

    This is the contract `metis_ifu_rsrf` and `metis_ifu_wavecal` depend on, so it is
    what stops a change to the tracing code from silently breaking the IFU cascade.
    """

    @staticmethod
    def read_back(table) -> list:
        # `read` never touches instance state, so calling it on the class is enough
        # and avoids having to construct a DataItem with a real frame behind it.
        return IfuDistortionTable.read(IfuDistortionTable, distortion_table=table)

    def test_round_trip_reproduces_trace_positions(self):
        traces = [
            Trace(m=k, pos=slice_coefficients(k), column_range=(6, 2042))
            for k in range(N_SLICES)
        ]

        read = self.read_back(traces_to_table(traces, degree=2))

        assert len(read) == len(traces)
        for t, (x, y) in zip(traces, read):
            assert x[0] == t.column_range[0]
            assert x[-1] == t.column_range[1] - 1
            np.testing.assert_allclose(y, np.polyval(t.pos, x), rtol=1e-12, atol=1e-9)

    def test_round_trip_from_a_traced_frame(self):
        """End to end: trace a frame, serialise, read back, compare."""
        frame, _ = synthetic_frame()
        traces = trace(frame, **TRACE_PARAMETERS)

        read = self.read_back(traces_to_table(traces, degree=2))

        assert len(read) == len(traces)
        for t, (x, y) in zip(traces, read):
            np.testing.assert_allclose(y, t.y_at_x(x), rtol=1e-12, atol=1e-9)

    def test_empty_table_reads_back_empty(self):
        """No traces detected must produce a valid, empty table."""
        table = traces_to_table([], degree=2)

        assert len(table) == 0
        assert self.read_back(table) == []

    def test_table_has_expected_columns(self):
        table = traces_to_table([Trace(m=0, pos=slice_coefficients(0),
                                      column_range=(6, 2042))], degree=2)

        assert set(table.column_names) == {'orders', 'column_range'}
        assert len(table) == 1

    def test_degree_mismatch_is_rejected(self):
        """Serialising with the wrong degree must fail loudly, not truncate."""
        traces = [Trace(m=0, pos=slice_coefficients(0), column_range=(6, 2042))]

        with pytest.raises(ValueError, match='coefficients'):
            traces_to_table(traces, degree=4)
