"""
Unit tests for the two-dimensional IFU wavelength solution.

Each test exercises one property: recovery of a known tilted wavelength solution,
grouping of a line across cross-dispersion offsets, wavelength assignment by both
strategies, automatic reduction of the polynomial degree when the lines cannot constrain
it, the fallback path, and the wavelength map's contract with `metis_ifu_rsrf` (microns,
and exactly zero outside the slices).
"""
import numpy as np
import pytest

from pymetis.drl.lines import Line
from pymetis.drl.trace_model import Trace
from pymetis.drl.wavecal import (assign_wavelengths, build_wavelength_map,
                                 extract_offset_spectra, fit_wavelength_solution,
                                 group_lines, linear_solution, solve_slice)
from pymetis.engine.core.functions.polyfit2d import polyval2d_safe

NROW = NCOL = 2048
WAVELENGTH_START = 3.5565
WAVELENGTH_END = 3.5823
DISPERSION = (WAVELENGTH_END - WAVELENGTH_START) / (NCOL - 1)
# Shift of a line's centroid, in pixels, per pixel of distance from the slice mid-line
TILT = -0.012
SLICE_HEIGHT = 114.0
LASERS = (3.5600, 3.5680, 3.5760)


def true_solution():
    """Coefficients of `lambda = start + dispersion * (x + tilt * dy)`."""
    coefficients = np.zeros((2, 2))
    coefficients[0, 0] = WAVELENGTH_START
    coefficients[1, 0] = DISPERSION
    coefficients[0, 1] = DISPERSION * TILT
    return coefficients


def slice_trace(index=0):
    return Trace(m=index,
                 pos=np.array([-2.0e-6, 3.0e-3, 193.0 + index * 127.0]),
                 column_range=(6, 2042),
                 height=SLICE_HEIGHT)


def synthetic_frame(trace, lasers=LASERS, amplitude=6000.0, noise=20.0, seed=23):
    """A detector frame carrying tilted laser lines within one slice."""
    rng = np.random.default_rng(seed)
    columns = np.arange(NCOL)
    rows = np.arange(NROW)[:, None]

    centre = np.polyval(trace.pos, columns)
    offset = rows - centre[None, :]
    inside = np.abs(offset) <= SLICE_HEIGHT / 2

    frame = np.full((NROW, NCOL), 200.0)
    for wavelength in lasers:
        position = (wavelength - WAVELENGTH_START) / DISPERSION - TILT * offset
        frame += np.where(
            inside,
            amplitude * np.exp(-0.5 * ((columns[None, :] - position) / 3.0) ** 2),
            0.0,
        )

    return frame + rng.normal(0, noise, frame.shape)


class TestExtractOffsetSpectra:
    def test_returns_one_spectrum_per_offset(self):
        trace = slice_trace()

        spectra, offsets = extract_offset_spectra(synthetic_frame(trace), trace,
                                                  height=SLICE_HEIGHT, n_offsets=7)

        assert spectra.shape == (7, NCOL)
        assert offsets.shape == (7,)

    def test_offsets_are_centred_on_the_mid_line(self):
        trace = slice_trace()

        _, offsets = extract_offset_spectra(synthetic_frame(trace), trace,
                                            height=SLICE_HEIGHT, n_offsets=5)

        assert np.mean(offsets) == pytest.approx(0.0)
        assert np.all(np.abs(offsets) <= SLICE_HEIGHT / 2)

    def test_masks_columns_outside_the_trace_range(self):
        trace = slice_trace()

        spectra, _ = extract_offset_spectra(synthetic_frame(trace), trace,
                                            height=SLICE_HEIGHT, n_offsets=5)

        assert np.all(spectra.mask[:, :trace.column_range[0]])
        assert np.all(spectra.mask[:, trace.column_range[1]:])
        assert spectra[:, trace.column_range[0]:trace.column_range[1]].count() > 0

    def test_masks_a_slice_running_off_the_detector(self):
        """A slice near the edge must be reported missing, not silently truncated."""
        trace = Trace(m=0, pos=np.array([0.0, 0.0, 2.0]), column_range=(6, 2042),
                      height=SLICE_HEIGHT)

        spectra, _ = extract_offset_spectra(np.zeros((NROW, NCOL)), trace,
                                            height=SLICE_HEIGHT, n_offsets=5)

        assert spectra[0].count() == 0


class TestGroupLines:
    def test_groups_one_line_across_offsets(self):
        lines = [Line(position=500.0 + 0.1 * i, fwhm=7.0, height=100.0, offset=float(i))
                 for i in range(5)]

        groups = group_lines(lines, tolerance=3.0)

        assert len(groups) == 1
        assert len(groups[0]) == 5

    def test_separates_distinct_lines(self):
        lines = ([Line(position=500.0, fwhm=7.0, height=100.0, offset=float(i))
                  for i in range(5)]
                 + [Line(position=900.0, fwhm=7.0, height=100.0, offset=float(i))
                    for i in range(5)])

        groups = group_lines(lines, tolerance=3.0)

        assert len(groups) == 2
        assert all(len(group) == 5 for group in groups)

    def test_never_takes_two_lines_from_one_offset(self):
        """Two detections at the same offset are different lines by definition."""
        lines = [Line(position=500.0, fwhm=7.0, height=100.0, offset=0.0),
                 Line(position=500.5, fwhm=7.0, height=100.0, offset=0.0)]

        groups = group_lines(lines, tolerance=3.0)

        assert len(groups) == 2

    def test_groups_are_ordered_by_position(self):
        lines = [Line(position=p, fwhm=7.0, height=100.0, offset=0.0)
                 for p in (900.0, 300.0, 1500.0)]

        groups = group_lines(lines)

        positions = [np.mean([m.position for m in g]) for g in groups]
        assert positions == sorted(positions)

    def test_tolerance_below_the_tilt_splits_a_line(self):
        lines = [Line(position=500.0 + 2.0 * i, fwhm=7.0, height=100.0, offset=float(i))
                 for i in range(5)]

        assert len(group_lines(lines, tolerance=0.5)) > 1


class TestAssignWavelengths:
    def make_groups(self, positions):
        return [[Line(position=p, fwhm=7.0, height=100.0, offset=0.0)]
                for p in positions]

    def test_matches_against_the_approximate_model(self):
        approximate = linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL)
        positions = [(w - WAVELENGTH_START) / DISPERSION for w in LASERS]
        groups = self.make_groups(positions)

        assigned = assign_wavelengths(groups, list(LASERS), approximate=approximate,
                                      tolerance=0.001)

        assert assigned == len(LASERS)
        for group, wavelength in zip(groups, LASERS):
            assert group[0].wavelength == pytest.approx(wavelength)

    def test_matches_in_order_without_a_model(self):
        """With equal counts, sorting by position and by wavelength agrees."""
        groups = self.make_groups([100.0, 800.0, 1600.0])

        assigned = assign_wavelengths(groups, list(LASERS))

        assert assigned == 3
        assert [g[0].wavelength for g in groups] == list(LASERS)

    def test_assigns_the_whole_group(self):
        groups = [[Line(position=500.0, fwhm=7.0, height=100.0, offset=float(i))
                   for i in range(4)]]

        assign_wavelengths(groups, [LASERS[0]])

        assert all(line.wavelength == LASERS[0] for line in groups[0])

    def test_tolerance_rejects_a_distant_match(self):
        approximate = linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL)
        groups = self.make_groups([100.0])

        assigned = assign_wavelengths(groups, [5.26], approximate=approximate,
                                      tolerance=1e-6)

        assert assigned == 0
        assert groups[0][0].wavelength is None

    def test_mismatched_counts_without_a_model_assign_nothing(self):
        groups = self.make_groups([100.0, 800.0])

        assert assign_wavelengths(groups, list(LASERS)) == 0

    def test_prefers_the_closest_pairing(self):
        """
        A wavelength must not claim a line that another wavelength fits better.

        With a candidate list containing near-duplicates -- as happens when the same
        list is offered to several detectors with overlapping coverage -- matching
        greedily in wavelength order labels lines with a neighbour's value, producing a
        confidently wrong solution rather than no solution.
        """
        approximate = linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL)
        positions = [(w - WAVELENGTH_START) / DISPERSION for w in LASERS]
        groups = self.make_groups(positions)

        # Each true wavelength is shadowed by a decoy 0.0007 um away, well inside the
        # tolerance, and the decoys sort first
        decoys = [w - 0.0007 for w in LASERS]
        assigned = assign_wavelengths(groups, list(LASERS) + decoys,
                                      approximate=approximate, tolerance=0.003)

        assert assigned == len(LASERS)
        for group, wavelength in zip(groups, LASERS):
            assert group[0].wavelength == pytest.approx(wavelength)

    def test_each_wavelength_is_used_at_most_once(self):
        approximate = linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL)
        positions = [(LASERS[0] - WAVELENGTH_START) / DISPERSION + shift
                     for shift in (0.0, 5.0, 10.0)]
        groups = self.make_groups(positions)

        assigned = assign_wavelengths(groups, [LASERS[0]], approximate=approximate,
                                      tolerance=0.003)

        assert assigned == 1
        assert sum(1 for g in groups if g[0].wavelength is not None) == 1

    def test_empty_input_is_handled(self):
        assert assign_wavelengths([], list(LASERS)) == 0
        assert assign_wavelengths(self.make_groups([1.0]), []) == 0


class TestFitWavelengthSolution:
    def lines_from_truth(self, wavelengths=LASERS, offsets=(-40, -20, 0, 20, 40)):
        coefficients = true_solution()
        lines = []
        for wavelength in wavelengths:
            for offset in offsets:
                position = (wavelength - WAVELENGTH_START) / DISPERSION - TILT * offset
                lines.append(Line(position=position, fwhm=7.0, height=100.0,
                                  offset=float(offset), wavelength=wavelength))
        # Sanity check that the constructed lines lie on the intended surface
        for line in lines:
            assert polyval2d_safe(line.position, line.offset, coefficients) == \
                pytest.approx(line.wavelength, abs=1e-12)
        return lines

    def test_recovers_the_known_solution(self):
        coefficients, rms, degree = fit_wavelength_solution(self.lines_from_truth(),
                                                            (1, 1))

        assert degree == (1, 1)
        assert rms == pytest.approx(0.0, abs=1e-12)
        np.testing.assert_allclose(coefficients, true_solution(), rtol=1e-6, atol=1e-15)

    def test_recovers_the_tilt_term(self):
        """The cross-dispersion term is the tilt, and must not come out zero."""
        coefficients, _, _ = fit_wavelength_solution(self.lines_from_truth(), (1, 1))

        assert coefficients[0, 1] == pytest.approx(DISPERSION * TILT, rel=1e-4)

    def test_reduces_degree_when_wavelengths_are_too_few(self):
        """Two wavelengths can only support a linear dispersion."""
        lines = self.lines_from_truth(wavelengths=LASERS[:2])

        _, _, degree = fit_wavelength_solution(lines, (4, 1))

        assert degree == (1, 1)

    def test_reduces_degree_when_offsets_are_too_few(self):
        lines = self.lines_from_truth(offsets=(0,))

        _, _, degree = fit_wavelength_solution(lines, (2, 3))

        assert degree[1] == 0

    def test_single_wavelength_cannot_constrain_dispersion(self):
        lines = self.lines_from_truth(wavelengths=LASERS[:1])

        coefficients, rms, _ = fit_wavelength_solution(lines, (2, 1))

        assert coefficients is None
        assert rms is None

    def test_no_identified_lines_yields_nothing(self):
        lines = [Line(position=100.0, fwhm=7.0, height=1.0, offset=0.0)]

        coefficients, rms, _ = fit_wavelength_solution(lines, (2, 1))

        assert coefficients is None
        assert rms is None


class TestSolveSlice:
    def solve(self, **kwargs):
        trace = slice_trace()
        frame = synthetic_frame(trace)
        options = dict(wavelengths=list(LASERS), height=SLICE_HEIGHT, degree=(2, 1),
                       n_offsets=7,
                       approximate=linear_solution(WAVELENGTH_START, WAVELENGTH_END,
                                                   NCOL),
                       match_tolerance=0.01)
        options.update(kwargs)
        return trace, frame, solve_slice(frame, trace, **options)

    def test_fits_from_measured_lines(self):
        _, _, solution = self.solve()

        assert not solution.fallback
        assert solution.coefficients is not None
        assert solution.n_identified == len(LASERS) * 7

    def test_solution_matches_the_truth_across_the_slice(self):
        _, _, solution = self.solve()

        x = np.linspace(200, 1900, 40)
        dy = np.linspace(-SLICE_HEIGHT / 2, SLICE_HEIGHT / 2, 9)
        grid_x, grid_dy = np.meshgrid(x, dy)

        expected = polyval2d_safe(grid_x, grid_dy, true_solution())
        # Better than a thousandth of a nanometre
        np.testing.assert_allclose(solution.evaluate(grid_x, grid_dy), expected,
                                   atol=1e-6)

    def test_residual_is_far_below_a_pixel_of_dispersion(self):
        _, _, solution = self.solve()

        assert solution.rms is not None
        assert solution.rms < 0.2 * DISPERSION

    def test_falls_back_when_no_line_is_present(self):
        trace = slice_trace()
        blank = np.full((NROW, NCOL), 200.0)
        approximate = linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL)

        solution = solve_slice(blank, trace, wavelengths=list(LASERS),
                               height=SLICE_HEIGHT, approximate=approximate)

        assert solution.fallback
        np.testing.assert_array_equal(solution.coefficients, approximate)
        assert solution.rms is None
        assert solution.n_identified == 0

    def test_without_a_fallback_there_is_no_solution(self):
        trace = slice_trace()

        solution = solve_slice(np.full((NROW, NCOL), 200.0), trace,
                               wavelengths=list(LASERS), height=SLICE_HEIGHT)

        assert solution.coefficients is None
        assert solution.evaluate(np.zeros(3), np.zeros(3)) is None


class TestBuildWavelengthMap:
    def build(self, fill=1.0):
        traces = [slice_trace(k) for k in range(3)]
        solutions = []
        for trace in traces:
            frame = synthetic_frame(trace)
            solutions.append(solve_slice(
                frame, trace, wavelengths=list(LASERS), height=SLICE_HEIGHT,
                degree=(2, 1), n_offsets=7,
                approximate=linear_solution(WAVELENGTH_START, WAVELENGTH_END, NCOL),
                match_tolerance=0.01,
            ))
        heights = [SLICE_HEIGHT * fill] * len(traces)
        return traces, build_wavelength_map((NROW, NCOL), traces, solutions, heights)

    def test_pixels_outside_the_slices_are_exactly_zero(self):
        """
        `metis_ifu_rsrf` treats zero as "no wavelength" and rejects those pixels, so
        anything not covered by a slice must be left untouched.
        """
        _, wavelength_map = self.build()

        assert wavelength_map[0, 0] == 0.0
        assert wavelength_map[-1, -1] == 0.0
        assert np.any(wavelength_map == 0.0)

    def test_values_are_in_microns_within_the_covered_range(self):
        _, wavelength_map = self.build()

        covered = wavelength_map[wavelength_map > 0]
        assert covered.size > 0
        assert covered.min() > WAVELENGTH_START - 0.01
        assert covered.max() < WAVELENGTH_END + 0.01

    def test_covers_the_expected_number_of_pixels(self):
        traces, wavelength_map = self.build()

        span = traces[0].column_range[1] - traces[0].column_range[0]
        expected = len(traces) * span * SLICE_HEIGHT
        assert np.count_nonzero(wavelength_map) == pytest.approx(expected, rel=0.02)

    def test_wavelength_varies_across_the_slice(self):
        """A tilted solution must not produce columns of constant wavelength."""
        traces, wavelength_map = self.build()

        column = wavelength_map[:, NCOL // 2]
        inside = column[column > 0]

        assert inside.size > 0
        assert inside.max() - inside.min() > 0

    def test_a_slice_without_a_solution_is_left_blank(self):
        trace = slice_trace()
        solution = solve_slice(np.full((NROW, NCOL), 200.0), trace,
                               wavelengths=list(LASERS), height=SLICE_HEIGHT)

        wavelength_map = build_wavelength_map((NROW, NCOL), [trace], [solution],
                                             [SLICE_HEIGHT])

        np.testing.assert_array_equal(wavelength_map, np.zeros((NROW, NCOL)))


class TestLinearSolution:
    def test_spans_the_requested_range(self):
        coefficients = linear_solution(3.5, 3.6, NCOL)

        assert polyval2d_safe(0.0, 0.0, coefficients) == pytest.approx(3.5)
        assert polyval2d_safe(float(NCOL - 1), 0.0, coefficients) == pytest.approx(3.6)

    def test_has_no_cross_dispersion_dependence(self):
        coefficients = linear_solution(3.5, 3.6, NCOL)

        assert polyval2d_safe(100.0, -50.0, coefficients) == \
            pytest.approx(polyval2d_safe(100.0, 50.0, coefficients))
