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
from pymetis.drl.wavecal import (MAX_TILT_DEGREE, SliceSolution, assign_wavelengths,
                                 build_wavelength_map, extract_collapsed_spectrum,
                                 extract_offset_spectra, fit_tilt_solution,
                                 fit_wavelength_solution, group_lines, linear_solution,
                                 solutions_from_table, solutions_to_table, solve_slice)
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


class TestExtractCollapsedSpectrum:
    def test_masks_columns_outside_the_trace_range(self):
        trace = slice_trace()

        spectrum = extract_collapsed_spectrum(synthetic_frame(trace), trace,
                                              height=SLICE_HEIGHT)

        assert spectrum.shape == (NCOL,)
        assert np.all(spectrum.mask[:trace.column_range[0]])
        assert np.all(spectrum.mask[trace.column_range[1]:])
        assert spectrum[trace.column_range[0]:trace.column_range[1]].count() > 0

    def test_tilt_correction_recovers_a_smeared_line(self):
        """
        Without correction, a strongly tilted line's signal lands on a different
        column in every row, so collapsing the height smears it away to nothing.
        Correcting for the tilt puts every row's contribution back on the same
        column before collapsing, recovering the line's full amplitude.
        """
        trace = Trace(m=0, pos=np.array([0.0, 0.0, 193.0]), column_range=(6, 2042),
                      height=SLICE_HEIGHT)
        big_tilt = -0.5  # far larger than the module's TILT, to make the effect obvious
        x0 = 900.0

        columns = np.arange(NCOL)
        rows = np.arange(NROW)[:, None]
        offset = rows - 193.0
        inside = np.abs(offset) <= SLICE_HEIGHT / 2
        position = x0 - big_tilt * offset
        frame = np.full((NROW, NCOL), 200.0)
        frame += np.where(inside,
                          6000.0 * np.exp(-0.5 * ((columns[None, :] - position) / 3.0) ** 2),
                          0.0)

        uncorrected = extract_collapsed_spectrum(frame, trace, height=SLICE_HEIGHT)
        corrected = extract_collapsed_spectrum(
            frame, trace, height=SLICE_HEIGHT,
            tilt_coefficients=np.array([[0.0, -big_tilt]]))

        assert corrected.max() > 10 * uncorrected.max()
        assert np.argmax(corrected) == pytest.approx(x0, abs=1)


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
    def lines_from_truth(self, wavelengths=LASERS):
        """Lines from a single, already tilt-corrected spectrum: all at offset zero."""
        lines = []
        for wavelength in wavelengths:
            position = (wavelength - WAVELENGTH_START) / DISPERSION
            lines.append(Line(position=position, fwhm=7.0, height=100.0,
                              offset=0.0, wavelength=wavelength))
        return lines

    def test_recovers_the_known_solution(self):
        coefficients, rms, degree = fit_wavelength_solution(self.lines_from_truth(), 1)

        assert degree == 1
        assert rms == pytest.approx(0.0, abs=1e-12)
        np.testing.assert_allclose(coefficients, [WAVELENGTH_START, DISPERSION],
                                   rtol=1e-6, atol=1e-15)

    def test_reduces_degree_when_wavelengths_are_too_few(self):
        """Two wavelengths can only support a linear dispersion."""
        lines = self.lines_from_truth(wavelengths=LASERS[:2])

        _, _, degree = fit_wavelength_solution(lines, 4)

        assert degree == 1

    def test_single_wavelength_cannot_constrain_dispersion(self):
        lines = self.lines_from_truth(wavelengths=LASERS[:1])

        coefficients, rms, _ = fit_wavelength_solution(lines, 2)

        assert coefficients is None
        assert rms is None

    def test_no_identified_lines_yields_nothing(self):
        lines = [Line(position=100.0, fwhm=7.0, height=1.0, offset=0.0)]

        coefficients, rms, _ = fit_wavelength_solution(lines, 2)

        assert coefficients is None
        assert rms is None


class TestFitTiltSolution:
    def groups_from_truth(self, references=(300.0, 900.0, 1500.0),
                          offsets=(-40, -20, 0, 20, 40), identify=False):
        """
        Groups of detections of the tilt model `position = x0 - TILT * offset`.

        No wavelength is assigned by default -- the tilt fit must not need one.
        """
        groups = []
        for index, x0 in enumerate(references):
            group = []
            for offset in offsets:
                position = x0 - TILT * offset
                wavelength = LASERS[index % len(LASERS)] if identify else None
                group.append(Line(position=position, fwhm=7.0, height=100.0,
                                  offset=float(offset), wavelength=wavelength))
            groups.append(group)
        return groups

    def test_recovers_the_tilt_from_unidentified_lines(self):
        groups = self.groups_from_truth()
        assert all(line.wavelength is None for group in groups for line in group)

        tilt_coefficients, degree = fit_tilt_solution(groups, 1)

        # The tilt-vs-offset degree follows the request (1); the tilt-vs-x degree is
        # independent of it and reaches MAX_TILT_DEGREE on its own, given enough groups
        assert degree == (MAX_TILT_DEGREE, 1)
        # h_1(x0) is the constant `-TILT`, independent of x0 in this synthetic model
        assert tilt_coefficients[0, 1] == pytest.approx(-TILT, rel=1e-6)
        assert tilt_coefficients[1, 1] == pytest.approx(0.0, abs=1e-9)
        # column 0 (the dy**0 term) is identically zero by construction
        np.testing.assert_array_equal(tilt_coefficients[:, 0], 0.0)

    def test_identification_makes_no_difference(self):
        unidentified = fit_tilt_solution(self.groups_from_truth(identify=False), 1)
        identified = fit_tilt_solution(self.groups_from_truth(identify=True), 1)

        np.testing.assert_array_equal(unidentified[0], identified[0])

    def test_reduces_degree_when_offsets_are_too_few(self):
        groups = self.groups_from_truth(offsets=(0,))

        _, degree = fit_tilt_solution(groups, 2)

        assert degree[1] == 0

    def test_caps_the_degree_regardless_of_request(self):
        """Neither degree may exceed `MAX_TILT_DEGREE`, however much is requested."""
        groups = self.groups_from_truth(references=(200.0, 600.0, 1000.0, 1400.0),
                                        offsets=(-40, -20, -10, 0, 10, 20, 40))

        _, degree = fit_tilt_solution(groups, 5)

        assert degree[0] <= MAX_TILT_DEGREE
        assert degree[1] <= MAX_TILT_DEGREE

    def test_no_groups_yields_nothing(self):
        tilt_coefficients, degree = fit_tilt_solution([], 1)

        assert tilt_coefficients is None
        assert degree == (0, 0)


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
        assert solution.wavelength_coefficients is not None
        # One identification per laser line, from the single collapsed spectrum --
        # unlike the tilt fit, this no longer multiplies by the number of offsets
        assert solution.n_identified == len(LASERS)

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
        np.testing.assert_array_equal(solution.wavelength_coefficients,
                                      np.asarray(approximate).reshape(-1))
        # A blank frame has no detections at any offset either, so the tilt fit has
        # nothing to work with
        assert solution.tilt_coefficients is None
        assert solution.rms is None
        assert solution.n_identified == 0

    def test_without_a_fallback_there_is_no_solution(self):
        trace = slice_trace()

        solution = solve_slice(np.full((NROW, NCOL), 200.0), trace,
                               wavelengths=list(LASERS), height=SLICE_HEIGHT)

        assert solution.wavelength_coefficients is None
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
        return traces, build_wavelength_map((NROW, NCOL), solutions, heights)

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

        wavelength_map = build_wavelength_map((NROW, NCOL), [solution], [SLICE_HEIGHT])

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


class TestSolutionsToTable:
    def test_tilt_columns_are_fixed_width_three(self):
        tilt_coefficients = np.array([[0.0, -0.02, 0.0], [0.0, 3e-5, 0.0]])
        fitted = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                              tilt_coefficients=tilt_coefficients, degree=(1, 1))
        fallback = SliceSolution(index=2, wavelength_coefficients=None,
                                 tilt_coefficients=None, degree=(0, 0))

        table = solutions_to_table([fitted, fallback])

        assert {'slit_poly_a', 'slit_poly_b', 'slit_poly_c'} <= set(table.column_names)
        for name in ('slit_poly_a', 'slit_poly_b', 'slit_poly_c'):
            assert len(table[name, 0][0]) == MAX_TILT_DEGREE + 1

    def test_populated_columns_match_the_tilt_coefficients(self):
        # Full (MAX_TILT_DEGREE+1, MAX_TILT_DEGREE+1) array, so every slot is a real
        # fitted value and none are the "not fit" NaN padding.
        tilt_coefficients = np.array([[0.0, -0.02, 0.001],
                                      [0.0, 3e-5, 0.0002],
                                      [0.0, 1e-8, 3e-6]])
        solution = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=tilt_coefficients, degree=(2, 2))

        table = solutions_to_table([solution])

        np.testing.assert_array_equal(table['slit_poly_a', 0][0], tilt_coefficients[:, 0])
        np.testing.assert_array_equal(table['slit_poly_b', 0][0], tilt_coefficients[:, 1])
        np.testing.assert_array_equal(table['slit_poly_c', 0][0], tilt_coefficients[:, 2])

    def test_unfit_slots_are_nan_not_zero(self):
        """A degree lower than `MAX_TILT_DEGREE` leaves the higher slots NaN, not 0."""
        tilt_coefficients = np.array([[0.0, -0.02, 0.0], [0.0, 3e-5, 0.0]])  # degree (1, 1)
        solution = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=tilt_coefficients, degree=(1, 1))

        table = solutions_to_table([solution])

        np.testing.assert_allclose(table['slit_poly_a', 0][0], [0.0, 0.0, np.nan],
                                   equal_nan=True)
        np.testing.assert_allclose(table['slit_poly_b', 0][0], [-0.02, 3e-5, np.nan],
                                   equal_nan=True)

    def test_no_tilt_is_all_nan(self):
        solution = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0))

        table = solutions_to_table([solution])

        for name in ('slit_poly_a', 'slit_poly_b', 'slit_poly_c'):
            assert all(np.isnan(v) for v in table[name, 0][0])

    def test_trace_columns_carry_the_solutions_trace(self):
        trace = Trace(m=3, slice=3, pos=np.array([0.001, 5.0, 200.0]),
                      column_range=(10, 2038))
        solution = SliceSolution(index=3, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0), trace=trace)

        table = solutions_to_table([solution])

        assert table['trace_nb', 0][0] == 3
        assert table['slice_nb', 0][0] == 3
        np.testing.assert_allclose(table['pos', 0][0], trace.pos)
        np.testing.assert_allclose(table['column_range', 0][0], list(trace.column_range))

    def test_a_solution_without_a_trace_still_fills_the_other_columns(self):
        """
        Losing the trace columns must not silently drop the whole row, as it once did
        when the row was skipped outright for lack of a trace.
        """
        solution = SliceSolution(index=7, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0), rms=2e-6)

        table = solutions_to_table([solution])

        assert table['trace_nb', 0][0] == 7
        assert table['slice_nb', 0][0] == 7
        assert all(np.isnan(v) for v in table['pos', 0][0])
        assert all(np.isnan(v) for v in table['column_range', 0][0])
        assert table['degree_dispersion', 0][0] == 1
        assert table['rms', 0][0] == pytest.approx(2e-6)


class TestSolutionsFromTable:
    """The inverse of `solutions_to_table`, for everything but `lines`."""

    def test_empty_table_reads_back_empty(self):
        assert solutions_from_table(solutions_to_table([])) == []

    def test_round_trip_preserves_a_fitted_solution(self):
        tilt_coefficients = np.array([[0.0, -0.02, 0.001],
                                      [0.0, 3e-5, 0.0002],
                                      [0.0, 1e-8, 3e-6]])
        solution = SliceSolution(index=5, wavelength_coefficients=np.array([3.5, 1.2e-5, 3e-9]),
                                 tilt_coefficients=tilt_coefficients, degree=(2, 2),
                                 rms=1.23e-6, fallback=False)

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        assert restored.index == solution.index
        np.testing.assert_allclose(restored.wavelength_coefficients,
                                   solution.wavelength_coefficients)
        np.testing.assert_allclose(restored.tilt_coefficients, tilt_coefficients)
        assert restored.degree == solution.degree
        assert restored.rms == pytest.approx(solution.rms)
        assert restored.fallback is False

    def test_round_trip_preserves_a_lower_degree_tilt(self):
        """A tilt that never reached `MAX_TILT_DEGREE` must not gain spurious terms."""
        tilt_coefficients = np.array([[0.0, -0.02], [0.0, 3e-5]])  # degree (1, 1)
        solution = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=tilt_coefficients, degree=(1, 1))

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        np.testing.assert_allclose(restored.tilt_coefficients, tilt_coefficients)

    def test_round_trip_preserves_a_fallback_solution(self):
        solution = SliceSolution(index=2, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0), rms=None,
                                 fallback=True)

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        np.testing.assert_allclose(restored.wavelength_coefficients,
                                   solution.wavelength_coefficients)
        assert restored.tilt_coefficients is None
        assert restored.rms is None
        assert restored.fallback is True

    def test_round_trip_preserves_an_empty_solution(self):
        solution = SliceSolution(index=3, wavelength_coefficients=None,
                                 tilt_coefficients=None, degree=(0, 0))

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        assert restored.wavelength_coefficients is None
        assert restored.tilt_coefficients is None

    def test_lines_do_not_survive_the_round_trip(self):
        """
        Only `n_lines`/`n_identified` counts are stored, not the `Line` measurements
        themselves, so there is nothing to reconstruct `lines` from.
        """
        lines = [Line(position=100.0 + i, fwhm=3.0, height=500.0, wavelength=4.7)
                for i in range(3)]
        solution = SliceSolution(index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0), lines=lines)
        table = solutions_to_table([solution])
        assert table['n_lines', 0][0] == 3
        assert table['n_identified', 0][0] == 3

        restored = solutions_from_table(table)[0]

        assert restored.lines == []
        assert restored.n_identified == 0

    def test_round_trip_preserves_the_trace(self):
        trace = Trace(m=4, slice=4, pos=np.array([1e-6, -0.002, 300.0]),
                      column_range=(8, 2040), bottom=np.array([0.0, 0.0, 250.0]),
                      top=np.array([0.0, 0.0, 350.0]))
        solution = SliceSolution(index=4, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0), trace=trace)

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        assert restored.trace is not None
        assert restored.trace.m == trace.m
        assert restored.trace.slice == trace.slice
        assert restored.trace.column_range == trace.column_range
        np.testing.assert_allclose(restored.trace.pos, trace.pos)
        np.testing.assert_allclose(restored.trace.bottom, trace.bottom)
        np.testing.assert_allclose(restored.trace.top, trace.top)

    def test_round_trip_of_a_solution_without_a_trace_leaves_it_none(self):
        solution = SliceSolution(index=9, wavelength_coefficients=np.array([3.5, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0))

        restored = solutions_from_table(solutions_to_table([solution]))[0]

        assert restored.trace is None
        assert restored.index == solution.index

    def test_round_trip_preserves_traces_independently_across_rows(self):
        """A table holding both traced and untraced solutions must not cross-contaminate."""
        traced = SliceSolution(
            index=1, wavelength_coefficients=np.array([3.5, 1e-5]),
            tilt_coefficients=None, degree=(1, 0),
            trace=Trace(m=1, slice=1, pos=np.array([2.0, 100.0]), column_range=(6, 2042)))
        untraced = SliceSolution(index=2, wavelength_coefficients=np.array([3.6, 1e-5]),
                                 tilt_coefficients=None, degree=(1, 0))

        restored = solutions_from_table(solutions_to_table([traced, untraced]))

        assert restored[0].trace is not None
        assert restored[0].trace.m == 1
        np.testing.assert_allclose(restored[0].trace.pos, [2.0, 100.0])
        assert restored[1].trace is None
        assert restored[1].index == 2
