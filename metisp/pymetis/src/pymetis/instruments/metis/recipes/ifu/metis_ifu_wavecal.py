"""
This file is part of the METIS Pipeline.
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
import copy
from typing import Literal

import cpl
from cpl.core import Msg

import numpy as np

from pymetis.engine.core.parameter import (ParameterList, ParameterEnum, ParameterRange,
                                           ParameterValue)
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameter, QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.engine.recipes import Recipe

from pymetis.drl.combine import combine_images
from pymetis.drl.trace import traces_from_table
from pymetis.drl.wavecal import (SliceSolution, build_wavelength_map, linear_solution,
                                 solutions_to_table, solve_slice)

from pymetis.instruments.metis.dataitems.wavecal import (IfuWavecalRaw, IfuWavecal,
                                                         IfuWavecalTab)
from pymetis.instruments.metis.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.instruments.metis.inputs import (MasterDarkInput, RawInput, DistortionTableInput, OptionalInputMixin,
                                              PersistenceMapInput, GainMapInput, LinearityInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab.darkimage import DarkImageProcessor

# Microns per Angstrom, for reporting the fit residual in the unit the DRLD declares
MICRON_IN_ANGSTROM = 1.0e4

# Fallback wavelength coverage per detector, in microns, retained from the original
# skeleton implementation. Used only when the frame header carries no grating setting.
FALLBACK_WAVELENGTH_START = (3.5565, 3.5284, 3.5275, 3.5557)
FALLBACK_WAVELENGTH_END = (3.5823, 3.5547, 3.5541, 3.5820)


class MetisIfuWavecalImpl(BandIfuMixin, DetectorIfuMixin, DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuWavecalRaw

        class MasterDarkInput(OptionalInputMixin, MasterDarkInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(OptionalInputMixin,GainMapInput):
            pass

        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class DistortionTableInput(DistortionTableInput):
            pass

    class ProductSet(PipelineProductSet):
        IfuWavecal = IfuWavecal
        IfuWavecalTab = IfuWavecalTab

    class Qc(QcParameterSet):
        class NLines(QcParameter):
            _name_template = "QC IFU WAVECAL NLINES"
            _type = int
            _unit = "1"
            _default = None
            _description_template = "Number of detected laser lines; should be constant"

        class Rms(QcParameter):
            _name_template = "QC IFU WAVECAL RMS"
            _type = float
            _unit = "Å"
            _default = None
            _description_template = "Root mean square of the residuals of the wavelength calibration fit"

        class PeakCounts(QcParameter):
            _name_template = "QC IFU WAVECAL PEAK CNTS"
            _type = float
            _unit = "counts"
            _default = None
            _description_template = "Peak counts of the laser line"

        class LineWidth(QcParameter):
            _name_template = "QC IFU WAVECAL LINE WIDTH"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "FWHM of the laser line as measured by fitting a Gaussian profile to it"
            _comment = "This fulfils METIS-6073"

    def _raw_header(self) -> cpl.core.PropertyList:
        """Primary header of the first raw frame, cached for keyword lookups."""
        if getattr(self, '_header_cache', None) is None:
            self._header_cache = cpl.core.PropertyList.load(
                self.inputset.raw.frameset[0].file, 0,
            )
        return self._header_cache

    def _laser_wavelengths(self) -> list[float]:
        """
        Wavelengths of the WCU laser lines expected in the frame, in microns.

        The DRLD states that "the wavelength of each line is known from the settings of
        the QCL", but specifies no input carrying them, so the header is consulted first
        and the recipe parameter serves as the fallback. Simulated data currently
        provides only `ESO SEQ WCU LASER<n> NAME` with no wavelength, so in practice the
        parameter is what is used.
        """
        header = self._raw_header()

        from_header = []
        for laser in range(1, 5):
            for keyword in (f"ESO SEQ WCU LASER{laser} WLEN",
                            f"ESO INS WCU LASER{laser} WLEN"):
                if keyword in header:
                    value = float(header[keyword].value)
                    if value > 0:
                        from_header.append(value)
                    break

        if from_header:
            Msg.info(self.__class__.__qualname__,
                     f"Laser wavelengths from header: {sorted(from_header)} um")
            return sorted(from_header)

        parameter = self.parameters[f"{self.name}.lines.wavelengths"].value
        wavelengths = sorted(float(part) for part in str(parameter).split(',')
                             if part.strip())

        Msg.info(self.__class__.__qualname__,
                 "No laser wavelength keywords in the header; using the "
                 f"{self.name}.lines.wavelengths parameter: {wavelengths} um")
        return wavelengths

    @staticmethod
    def _slice_spacings(traces: list, nrow: int) -> list[float]:
        """
        Distance to the nearest neighbouring slice, per trace, in pixels.

        Used only as the ceiling on how far a slice may be broadened: reaching past the
        neighbour would paint one slice's wavelengths onto another's pixels.
        """
        if len(traces) < 2:
            return [float(nrow)] * len(traces)

        mids = [0.5 * (t.column_range[0] + t.column_range[1]) for t in traces]
        centres = [float(t.y_at_x(mid)) for t, mid in zip(traces, mids)]

        spacings = []
        for i, centre in enumerate(centres):
            neighbours = [abs(centres[j] - centre)
                          for j in (i - 1, i + 1) if 0 <= j < len(centres)]
            spacings.append(min(neighbours))

        return spacings

    def _slice_heights(self, traces: list, nrow: int) -> list[float]:
        """
        Cross-dispersion extent to extract for each slice, in pixels.

        Prefers the extent measured on the distortion frame and stored in the table.
        Only when a table predates those columns does this fall back on the spacing of
        neighbouring slices scaled by `slices.fill_factor` -- that spacing includes the
        unilluminated gap between slices, and painting a wavelength into the gaps would
        make `metis_ifu_rsrf` treat those pixels as valid data.

        The measured extent is the width at half maximum, so the slice carries signal a
        little beyond it. `slices.broadening` widens the extracted band to cover those
        wings, because a pixel left without a wavelength is a pixel `metis_ifu_rsrf`
        cannot use. The broadened band is capped at the distance to the nearest
        neighbour, which is the point at which it would start describing the wrong slice.

        Returns
        -------
        list[float]
            One height per trace, in the order given.
        """
        measured = [t for t in traces if t.has_edges]
        broadening = float(self.parameters[f"{self.name}.slices.broadening"].value)
        spacings = self._slice_spacings(traces, nrow)

        def broadened(trace, spacing: float) -> float:
            """Measured extent, widened to catch the wings but not the neighbour."""
            mid = 0.5 * (trace.column_range[0] + trace.column_range[1])
            extent = float(trace.height_at_x(mid)) * (1.0 + broadening)
            return min(extent, spacing)

        if len(measured) == len(traces):
            heights = [broadened(t, s) for t, s in zip(traces, spacings)]
            Msg.info(self.__class__.__qualname__,
                     f"Using the measured slice extent from the distortion table for all "
                     f"{len(traces)} slices, broadened by {broadening:.0%} to "
                     f"a median of {float(np.median(heights)):.1f} px")
            return heights

        fill_factor = float(self.parameters[f"{self.name}.slices.fill_factor"].value)
        if measured:
            Msg.warning(self.__class__.__qualname__,
                        f"Only {len(measured)} of {len(traces)} slices carry a measured "
                        f"extent; falling back to the slice spacing scaled by "
                        f"fill_factor={fill_factor} for the rest")
        else:
            Msg.info(self.__class__.__qualname__,
                     f"The distortion table carries no measured slice extent, so the "
                     f"spacing scaled by fill_factor={fill_factor} is used instead")

        heights = []
        for t, spacing in zip(traces, spacings):
            if t.has_edges:
                heights.append(broadened(t, spacing))
            else:
                # fill_factor already shrinks the spacing deliberately, so broadening it
                # again would just undo that
                heights.append(fill_factor * (t.height if t.height else nrow / len(traces)))

        return heights

    def _approximate_solution(self,
                             detector: Literal[1, 2, 3, 4],
                             ncol: int) -> np.ndarray:
        """
        Approximate linear dispersion, used to identify lines and as the fallback.

        Derived from the grating setting when the header provides it, otherwise from the
        per-detector coverage the skeleton implementation hardcoded.
        """
        header = self._raw_header()
        width = float(self.parameters[f"{self.name}.dispersion.width"].value)

        if 'ESO INS WLEN CEN' in header:
            centre = float(header['ESO INS WLEN CEN'].value)
            # fixme: determine det location from the header, not the hardcoded order
            if detector in [1,3]:
                centre -= 0.5 * width
            else:
                centre += 0.5 * width
            Msg.info(self.__class__.__qualname__,
                        f"DET{detector}: Approximate linear dispersion from grating setting: "
                        f"{centre - width / 2:.4f} to {centre + width / 2:.4f} um")
            
            return linear_solution(centre - width / 2, centre + width / 2, ncol)

        return linear_solution(FALLBACK_WAVELENGTH_START[detector - 1],
                               FALLBACK_WAVELENGTH_END[detector - 1],
                               ncol)

    def _solve_parameters(self) -> dict:
        """Collect the line-fitting parameters from the recipe parameter list."""
        name = self.name
        return {
            'degree': (int(self.parameters[f"{name}.solution.degree_dispersion"].value),
                       int(self.parameters[f"{name}.solution.degree_spatial"].value)),
            'n_offsets': int(self.parameters[f"{name}.solution.offsets"].value),
            'match_tolerance':
                float(self.parameters[f"{name}.lines.match_tolerance"].value),
            'smoothing': float(self.parameters[f"{name}.lines.smoothing"].value),
            'min_snr': float(self.parameters[f"{name}.lines.min_snr"].value),
        }

    def _process_single_detector(self,
                                 detector: Literal[1, 2, 3, 4],
                                 method: str,
                                 wavelengths: list[float],
                                 solve_parameters: dict) -> dict:
        """
        Determine the wavelength solution for a single detector of the IFU.

        Follows the DRLD prescription: measure the line locations by Gaussian fit,
        compute the deviation from the approximate optical model, fit the per-slice
        solution lambda(x, y), and compute the wavelength map. Lines are measured at
        several cross-dispersion offsets within each slice, so the fit also captures
        their tilt with respect to the detector columns.

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4]
        method : str
            Method used to stack the raw exposures.
        wavelengths : list[float]
            Expected laser line wavelengths, in microns.
        solve_parameters : dict
            Keyword arguments for `pymetis.drl.wavecal.solve_slice`.

        Returns
        -------
        dict
            The wavelength map HDU and the measurements needed for quality control.
        """
        det = rf'DET{detector:1d}'

        raw_images = self.inputset.raw.use().load_data(extension=rf'{det}.DATA')
        combined_image = combine_images(raw_images, method)
        image = combined_image.as_array()
        nrow, ncol = image.shape

        # The distortion table gives the slice mid-lines and their
        # illuminated extent.
        distortion_table = self.inputset.distortion_table.load_data(extension=det)
        traces = traces_from_table(distortion_table)

        approximate = self._approximate_solution(detector, ncol)

        if not traces:
            Msg.warning(self.__class__.__qualname__,
                        f"{det}: the distortion table holds no slices, so no "
                        f"wavelength solution can be determined; its wavelength map "
                        f"will be empty")
            return {'HDU': self._wavelength_hdu(np.zeros((nrow, ncol)), det),
                    'solutions': [], 'lines': []}

        heights = self._slice_heights(traces, nrow)

        solutions = [
            solve_slice(image, trace,
                        wavelengths=wavelengths,
                        height=height,
                        approximate=approximate,
                        **solve_parameters)
            for trace, height in zip(traces, heights)
        ]

        fitted = sum(1 for s in solutions if not s.fallback)
        if fitted == 0:
            Msg.warning(self.__class__.__qualname__,
                        f"{det}: no slice yielded a wavelength solution from measured "
                        f"lines; falling back to the approximate linear dispersion for "
                        f"all {len(solutions)} slices")
        else:
            Msg.info(self.__class__.__qualname__,
                     f"{det}: fitted {fitted} of {len(solutions)} slices from "
                     f"measured lines")

        wavelength_map = build_wavelength_map((nrow, ncol), traces, solutions, heights)

        return {
            'HDU': self._wavelength_hdu(wavelength_map, det),
            'TABLE': Hdu(create_dummy_header(EXTNAME=det),
                         solutions_to_table(solutions),
                         name=det),
            'solutions': solutions,
            'lines': [line for s in solutions for line in s.lines],
        }

    @staticmethod
    def _wavelength_hdu(wavelength_map: np.ndarray, det: str) -> Hdu:
        """
        Wrap a wavelength map as a product extension.

        The map is in microns and is exactly zero outside the slices. Both are part of
        the product contract: `metis_ifu_rsrf` reads these values as microns and treats
        zero as "no wavelength here".
        """
        return Hdu(create_dummy_header(EXTNAME=det),
                   cpl.core.Image(wavelength_map),
                   name=det)

    def process(self) -> set[DataItem]:
        method = self.parameters[f"{self.name}.stacking.method"].value
        wavelengths = self._laser_wavelengths()
        solve_parameters = self._solve_parameters()

        output = [self._process_single_detector(det, method, wavelengths, solve_parameters)
                  for det in [1, 2, 3, 4]]

        primary_header = cpl.core.PropertyList()
        primary_header.append(self._collect_qc(output))

        product_wavecal = self.ProductSet.IfuWavecal(
            primary_header,
            *[out['HDU'] for out in output],
        )
        product_wavecal_tab = self.ProductSet.IfuWavecalTab(
            copy.deepcopy(primary_header),
            *[out['TABLE'] for out in output],
        )

        return {product_wavecal, product_wavecal_tab}

    def _collect_qc(self, output: list[dict]) -> cpl.core.PropertyList:
        """
        Summarise the line measurements into quality control parameters.

        `NLINES` counts identified lines across every slice and detector. `RMS` pools the
        per-slice fit residuals, converted to Angstrom as the DRLD declares. `PEAK CNTS`
        and `LINE WIDTH` are the median amplitude and median fitted width of the
        measured lines.

        A parameter with nothing behind it is omitted from the header rather than filled
        with a placeholder, so `RMS` is absent whenever every slice fell back to the
        approximate model.
        """
        solutions: list[SliceSolution] = [s for out in output for s in out['solutions']]
        identified = [line for out in output for line in out['lines']
                      if line.wavelength is not None]

        n_lines = sum(s.n_identified for s in solutions)

        residuals = [s.rms for s in solutions if s.rms is not None]
        rms = (float(np.sqrt(np.mean(np.square(residuals)))) * MICRON_IN_ANGSTROM
               if residuals else None)

        peak_counts = float(np.median([line.height for line in identified])) \
            if identified else None
        line_width = float(np.median([line.fwhm for line in identified])) \
            if identified else None

        Msg.info(self.__class__.__qualname__, f"QC IFU WAVECAL NLINES = {n_lines}")
        Msg.info(self.__class__.__qualname__, f"QC IFU WAVECAL RMS = {rms}")
        Msg.info(self.__class__.__qualname__, f"QC IFU WAVECAL PEAK CNTS = {peak_counts}")
        Msg.info(self.__class__.__qualname__, f"QC IFU WAVECAL LINE WIDTH = {line_width}")

        if rms is None:
            Msg.warning(self.__class__.__qualname__,
                        "No wavelength solution was fitted from measured lines, so the "
                        "wavelength maps rest on the approximate dispersion model and "
                        "QC RMS is not reported")

        qc = [self.Qc.NLines(n_lines)]
        if rms is not None:
            qc.append(self.Qc.Rms(rms))
        if peak_counts is not None:
            qc.append(self.Qc.PeakCounts(peak_counts))
        if line_width is not None:
            qc.append(self.Qc.LineWidth(line_width))

        return self.collect_qc_parameters(*qc)


class MetisIfuWavecal(Recipe):
    _name: str = "metis_ifu_wavecal"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Determine the wavelength calibration of the IFU"
    _description: str = (
        "Measures the WCU laser lines in each spatial slice of the IFU and fits a "
        "two-dimensional wavelength solution per slice, producing an image holding the "
        "wavelength of every pixel.\n"
        "Lines are measured at several cross-dispersion offsets within each slice, so "
        "the solution also accounts for their tilt with respect to the detector "
        "columns. Where too few lines can be identified to constrain a fit, the "
        "approximate dispersion model is used instead and this is reported.\n"
        "\n"
        "KNOWN DEVIATIONS FROM THE DRLD\n"
        "1. Second product. IFU_WAVECAL_TAB accompanies the wavelength map, holding the "
        "per-slice fit: coefficients, degrees, line counts, residual, and whether the "
        "slice was fitted from measured lines or fell back on the approximate model. "
        "Agreed in review (PR #220); the map alone cannot express that last "
        "distinction, which otherwise survives only as a log warning.\n"
        "2. Solution form. The solution is stored as lambda = f(x, dy) per slice. "
        "Storing lambda = f(x) together with a separate slit tilt f(dy) may suit "
        "rectification better, and is an open question from the same review; it waits "
        "on slit tilt being determined at all, which needs a line-rich frame.\n"
        "\n"
        "The extracted slice extent comes from the edges measured by "
        "metis_ifu_distortion where the table carries them, widened by "
        "slices.broadening so that pixels in the wings still receive a wavelength; "
        "metis_ifu_rsrf treats a pixel without one as invalid. Older tables without "
        "those columns fall back on slices.fill_factor."
    )

    _algorithm = """Stack the raw exposures and take the slice geometry from the distortion table.
        Extract spectra at several cross-dispersion offsets along each slice.
        Measure line locations by Gaussian fit, giving sub-pixel centroid, width and height.
        Group detections of the same line across offsets and identify them against the
        expected laser wavelengths, using the optical model to resolve the assignment.
        Fit the wavelength solution lambda(x, y) per slice as a low-order polynomial,
        reducing its degree where the available lines cannot constrain it.
        Evaluate the solution over each slice to build the wavelength map.

        Line detection and Gaussian centroiding are adapted from PyReduce (Piskunov &
        Valenti 2002, Piskunov, Wehrhahn & Marquart 2021), as prescribed by the DRLD."""
    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
        ParameterValue(
            name=f"{_name}.lines.wavelengths",
            context=_name,
            description="Comma-separated wavelengths of the expected WCU laser lines, "
                        "in microns. Used only when the frame header carries no laser "
                        "wavelength keywords. Defaults to the three METIS WCU lasers, "
                        "the middle one being the tuneable QCL",
            default="4.7, 4.71, 4.72, 4.73, 4.74, 4.75, 4.76",
        ),
        ParameterValue(
            name=f"{_name}.lines.match_tolerance",
            context=_name,
            description="Largest wavelength difference, in microns, allowed when "
                        "matching a measured line to an expected laser wavelength",
            default=0.004,
        ),
        ParameterValue(
            name=f"{_name}.lines.smoothing",
            context=_name,
            description="Standard deviation, in pixels, of the Gaussian smoothing "
                        "applied before line detection. Larger values reach fainter "
                        "lines at the cost of blending close ones",
            default=0.1,
        ),
        ParameterValue(
            name=f"{_name}.lines.min_snr",
            context=_name,
            description="Minimum line peak height in units of the noise standard "
                        "deviation. Zero disables the test",
            default=5.0,
        ),
        ParameterRange(
            name=f"{_name}.solution.degree_dispersion",
            context=_name,
            description="Polynomial degree of the wavelength solution along the "
                        "dispersion direction. Reduced automatically when too few "
                        "distinct wavelengths are identified to constrain it",
            default=2,
            min=1,
            max=5,
        ),
        ParameterRange(
            name=f"{_name}.solution.degree_spatial",
            context=_name,
            description="Polynomial degree of the wavelength solution across the slice. "
                        "A degree of 1 captures a linear line tilt",
            default=1,
            min=0,
            max=2,
        ),
        ParameterRange(
            name=f"{_name}.solution.offsets",
            context=_name,
            description="Number of cross-dispersion positions at which each slice is "
                        "sampled. Limits the achievable spatial degree to one less",
            default=3,
            min=1,
            max=25,
        ),
        ParameterRange(
            name=f"{_name}.slices.fill_factor",
            context=_name,
            description="Illuminated fraction of the spacing between neighbouring "
                        "slices. Only used as a fallback, for distortion tables written "
                        "before the measured slice edges were added to them; where those "
                        "edges are present the extent is read from the table instead. "
                        "The default reproduces the 114 pixel slice height of the METIS "
                        "IFU simulated data",
            default=0.9,
            min=0.1,
            max=1.0,
        ),
        ParameterRange(
            name=f"{_name}.slices.broadening",
            context=_name,
            description="Fraction by which to widen the measured slice extent when "
                        "painting the wavelength map. The measurement is a width at half "
                        "maximum, so the slice still carries signal beyond it, and a "
                        "pixel with no wavelength is one metis_ifu_rsrf cannot use. "
                        "Capped at the distance to the neighbouring slice. Ignored where "
                        "the extent came from slices.fill_factor instead",
            default=0.05,
            min=0.0,
            max=1.0,
        ),
        ParameterValue(
            name=f"{_name}.dispersion.width",
            context=_name,
            description="Instantaneous wavelength coverage of one detector, in microns, "
                        "used with the grating central wavelength to build the "
                        "approximate dispersion model",
            default=0.036,
        ),
    ])

    Impl = MetisIfuWavecalImpl
