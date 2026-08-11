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

from typing import Literal

import cpl
from cpl.core import Msg
import numpy as np

from pymetis.engine.core.parameter import (ParameterList, ParameterEnum, ParameterRange,
                                           ParameterValue)
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet, QcParameter
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.drl.combine import combine_images
from pymetis.drl.trace import (measure_trace_edges, measure_trace_fwhm, trace,
                               traces_to_table)

from pymetis.instruments.metis.inputs import (RawInput, MasterDarkInput, OptionalInputMixin, PersistenceMapInput,
                                              PinholeTableInput, GainMapInput, LinearityInput)
from pymetis.instruments.metis.mixins import DetectorIfuMixin
from pymetis.instruments.metis.dataitems.distortion import IfuDistortionRaw, IfuDistortionTable, IfuDistortionReduced
from pymetis.instruments.metis.dataitems.rsrf.raw import IfuRsrfRaw
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab.darkimage import DarkImageProcessor

class MetisIfuDistortionImpl(DetectorIfuMixin, DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class MasterDarkInput(MasterDarkInput):
            pass

        class PinholeTableInput(PinholeTableInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass


        class RawInput(RawInput):
            Item = IfuDistortionRaw

        class TraceReferenceInput(OptionalInputMixin, RawInput):
            """
            EXPERIMENTAL, and a deviation from the DRLD input list for this recipe.

            The DRLD gives this recipe only the multi-pinhole exposure, but the algorithm
            it prescribes (critical algorithm 5b, "use the same algorithms as for LSS
            (PyReduce)") locates slices by thresholding a *continuum-illuminated* frame
            and fitting a polynomial to each connected cluster of illuminated pixels. A
            pinhole exposure contains isolated spots, not the continuous traces that
            algorithm needs: on simulated data its largest connected feature is a few
            pixels, against slice-long bands of tens of thousands of pixels in an
            `IFU_RSRF_RAW` frame.

            Supplying an `IFU_RSRF_RAW` frame here is therefore optional and purely
            additive. When present it is traced instead of the pinhole exposure; when
            absent the recipe behaves exactly as before, so a DRLD-conformant set of
            frames still runs. Resolving this properly needs a decision on the recipe's
            input list, and a matching EDPS workflow association.
            """
            Item = IfuRsrfRaw

    class ProductSet(PipelineProductSet):
        DistortionTable = IfuDistortionTable
        DistortionReduced = IfuDistortionReduced

    class Qc(QcParameterSet):
        class Rms(QcParameter):
            _name_template = "QC IFU DISTORT RMS"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "Root mean square deviation between measured position and model"

        class Fwhm(QcParameter):
            _name_template = "QC IFU DISTORT FWHM"
            _type = float
            _unit = "pixels"
            _default = None
            _description_template = "Measure FWHM of spots"

        class NSpots(QcParameter):
            _name_template = "QC IFU DISTORT NSPOTS"
            _type = int
            _unit = "1"
            _default = None
            _description_template = "Number of identified spots"

    @staticmethod
    def _degree_or_best(value: str) -> int | Literal['best']:
        """
        Interpret a polynomial degree that may also be the string `best`.

        `trace` lets a degree be chosen per cluster rather than fixed, which
        `pyreduce`'s METIS IFU configuration asks for. CPL parameters are singly
        typed, so the choice travels as a string.
        """
        text = str(value).strip()
        if text == 'best':
            return 'best'

        try:
            return int(text)
        except ValueError:
            raise cpl.core.IllegalInputError(
                f"Expected an integer or 'best', but got {value!r}"
            ) from None

    def _trace_parameters(self) -> dict:
        """Collect the tracing parameters from the recipe parameter list."""
        name = self.name
        return {
            'degree': int(self.parameters[f"{name}.trace.degree"].value),
            'degree_before_merge': self._degree_or_best(
                self.parameters[f"{name}.trace.degree_before_merge"].value),
            'min_cluster': int(self.parameters[f"{name}.trace.min_cluster"].value),
            'min_width': float(self.parameters[f"{name}.trace.min_width"].value),
            'filter_x': int(self.parameters[f"{name}.trace.filter_x"].value),
            'filter_y': int(self.parameters[f"{name}.trace.filter_y"].value),
            'filter_type': str(self.parameters[f"{name}.trace.filter_type"].value),
            'noise': float(self.parameters[f"{name}.trace.noise"].value),
            'noise_relative':
                float(self.parameters[f"{name}.trace.noise_relative"].value),
            'border_width': int(self.parameters[f"{name}.trace.border_width"].value),
            'auto_merge_threshold':
                float(self.parameters[f"{name}.trace.auto_merge_threshold"].value),
            'merge_min_threshold':
                float(self.parameters[f"{name}.trace.merge_min_threshold"].value),
            'sigma': float(self.parameters[f"{name}.trace.sigma"].value),
        }

    #: WCU focal-plane mask position recorded for a frame taken with no mask in the beam
    OPEN_MASK = 'open'

    def _continuum_frames(self, reference) -> list[int]:
        """
        Indices of the trace-reference frames that are continuum-illuminated.

        `IFU_RSRF_RAW` classifies both the continuum flat and the pinhole-grid exposure:
        they share `DPR TYPE=RSRF`, `TECH=IFU` and `CATG=CALIB`, and the only keyword
        separating them is the WCU mask position, `ESO INS OPTI20 POSNAME` (`open` versus
        `grid_lm`). Tracing a grid frame here would defeat the purpose, since its
        cross-dispersion profile has the width of a spot rather than of a slice, so the
        grid frames are dropped rather than averaged in.

        Returns
        -------
        list[int]
            Positions within `reference.frameset`, empty if none qualify.
        """
        if not reference.frameset:
            return []

        continuum = []
        for index, frame in enumerate(reference.frameset):
            header = cpl.core.PropertyList.load(frame.file, 0)
            keyword = 'ESO INS OPTI20 POSNAME'

            if keyword not in header:
                Msg.warning(self.__class__.__qualname__,
                            f"{frame.file}: no {keyword}, so it cannot be told apart "
                            f"from a pinhole-grid exposure; not used for tracing")
                continue

            if str(header[keyword].value).strip() == self.OPEN_MASK:
                continuum.append(index)

        dropped = len(reference.frameset) - len(continuum)
        if dropped:
            Msg.info(self.__class__.__qualname__,
                     f"Ignoring {dropped} trace-reference frame(s) taken through a "
                     f"focal-plane mask; only unmasked frames show whole slices")

        return continuum

    def _process_single_detector(self,
                                 detector: Literal[1, 2, 3, 4],
                                 method: str,
                                 trace_parameters: dict) -> dict:
        """
        Determine the geometric distortion for a single detector of the IFU.

        The raw exposures are stacked, then the spatial slices of the image slicer are
        located by thresholding against a smoothed local background and fitting a
        polynomial mid-line to each resulting cluster of illuminated pixels. This is
        the algorithm the DRLD prescribes for IFU distortion (critical algorithm 5b).

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4] # FixMe: Maybe make this fully customizable for any detector count?
        method : str
            Method used to stack the raw exposures.
        trace_parameters : dict
            Keyword arguments for `pymetis.drl.trace.trace`.

        Returns
        -------
        dict
            The distortion table and stacked image HDUs, plus the quantities needed
            for the recipe's quality control parameters.
        """
        det = rf'{detector:1d}'
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')
        combined_image = combine_images(raw_images, method)

        # Trace the continuum frame when one was supplied, since a pinhole exposure holds
        # no continuous slices to trace. See TraceReferenceInput for why this is optional.
        reference = self.inputset.trace_reference
        continuum = self._continuum_frames(reference)

        if continuum:
            reference_images = reference.use().load_data(extension=rf'DET{det}.DATA')
            selected = cpl.core.ImageList()
            for index in continuum:
                selected.append(reference_images[index])

            trace_image = combine_images(selected, method).as_array()
            Msg.info(self.__class__.__qualname__,
                     f"DET{det}: tracing {len(continuum)} of "
                     f"{len(reference.frameset)} {reference.Item.name()} frames rather "
                     f"than the pinhole exposure")
        else:
            trace_image = combined_image.as_array()

        # CPL and HDRL have no order tracing, so the fitting is done in numpy
        traces = trace(trace_image, **trace_parameters)

        if not traces:
            Msg.warning(self.__class__.__qualname__,
                        f"No traces detected on DET{det}; "
                        f"its distortion table will be empty")

        # Measure the illuminated extent on the same frame the traces came from, so that
        # consumers read the aperture off the table instead of guessing it back from the
        # spacing between mid-lines.
        #
        # Only a continuum-illuminated frame can give this. A pinhole exposure lights a
        # row of spots rather than the whole slice, so its cross-dispersion profile has
        # the width of a spot -- about 3 px on the simulated data, against a slice height
        # of order 114 px. Measuring it there would hand `metis_ifu_wavecal` an aperture
        # 40x too small, which is worse than the spacing estimate it falls back on. The
        # edges are therefore left unset unless a continuum reference frame was supplied.
        if reference.frameset:
            measure_trace_edges(trace_image, traces, degree=trace_parameters['degree'])
        else:
            Msg.info(self.__class__.__qualname__,
                     f"DET{det}: tracing a pinhole exposure, so the slice extent cannot "
                     f"be measured from it; the distortion table will carry no edges and "
                     f"consumers will fall back on the slice spacing")

        # `QC IFU DISTORT NSPOTS` and `FWHM` are defined by the DRLD on the *pinhole*
        # exposure: the number of identified spots, and a spot width that "gives an
        # indication of the variation of spectral resolution across the field of view".
        # Neither is a property of the continuum frame, whose features are slices tens of
        # pixels tall, so when the solution came from a continuum reference the pinhole
        # exposure is traced separately for these two numbers. With no reference frame
        # both come from the one trace already done.
        if continuum:
            pinhole_image = combined_image.as_array()
            spots = trace(pinhole_image, **trace_parameters)
        else:
            pinhole_image, spots = trace_image, traces

        table = traces_to_table(traces, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        return {
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
            'residuals': [t.residual for t in traces if t.residual is not None],
            # The guard and the RMS follow the solution; the spot count and width follow
            # the pinhole exposure, which is what the DRLD defines them on
            'n_traces': len(traces),
            'n_spots': len(spots),
            'fwhm': measure_trace_fwhm(pinhole_image, spots),
        }

    def process(self) -> set[DataItem]:
        method = self.parameters[f"{self.name}.stacking.method"].value
        trace_parameters = self._trace_parameters()

        output = [self._process_single_detector(det, method, trace_parameters)
                  for det in [1, 2, 3, 4]]

        header_table = create_dummy_header()
        header_table.append(self._collect_qc(output))
        self._verify_any_trace_found(output)

        header_reduced = create_dummy_header()

        product_distortion = self.ProductSet.DistortionTable(
            header_table,
            *[out['TABLE'] for out in output],
        )
        product_distortion_reduced = self.ProductSet.DistortionReduced(
            header_reduced,
            *[out['IMAGE'] for out in output],
        )

        return {product_distortion, product_distortion_reduced}

    def _verify_any_trace_found(self, output: list[dict]) -> None:
        """
        Refuse to emit a distortion table that describes no slice at all.

        Such a table is not a degraded calibration but an unusable one:
        `metis_ifu_wavecal` turns it into an all-zero wavelength map and
        `metis_ifu_rsrf` only fails on that two recipes later, far from the frames that
        actually caused it. Stopping here names the culprit instead.

        Raises
        ------
        cpl.core.DataNotFoundError
            If no trace was found on any of the four detectors.
        """
        if any(out['n_traces'] > 0 for out in output):
            return

        raise cpl.core.DataNotFoundError(
            f"No slice was traced on any of the four detectors, so the distortion table "
            f"would be empty and no downstream recipe could use it. Either the raw "
            f"frames carry no usable illumination, or the tracing parameters do not fit "
            f"them: check {self.name}.trace.noise (the absolute threshold above the "
            f"local background) and {self.name}.trace.min_cluster against the actual "
            f"signal level and the detector size."
        )

    def _collect_qc(self, output: list[dict]) -> cpl.core.PropertyList:
        """
        Summarise the per-detector tracing results into quality control parameters.

        `NSPOTS` counts the spots identified in the pinhole exposure across the whole
        detector array, and `FWHM` is their median width, which indicates how the
        spectral resolution varies across the field of view. Both are measured on the
        pinhole frame even when the distortion solution itself came from a continuum
        reference, since neither quantity exists on a continuum frame: its features are
        slices, and counting those would report the slice count under a keyword that the
        DRLD defines as a number of spots.

        `RMS` pools the per-trace deviations of the measured mid-line from the fitted
        one, and so follows the solution rather than the pinhole exposure.

        `RMS` and `FWHM` are undefined when no trace was detected. Their keywords are
        then left out of the header altogether, rather than filled with a placeholder
        that a consumer could mistake for a measurement.
        """
        n_spots = sum(out['n_spots'] for out in output)

        residuals = [r for out in output for r in out['residuals']]
        rms = float(np.sqrt(np.mean(np.square(residuals)))) if residuals else None

        fwhms = [out['fwhm'] for out in output if out['fwhm'] is not None]
        fwhm = float(np.median(fwhms)) if fwhms else None

        qc = [self.Qc.NSpots(n_spots)]
        if rms is not None:
            qc.append(self.Qc.Rms(rms))
        if fwhm is not None:
            qc.append(self.Qc.Fwhm(fwhm))

        Msg.info(self.__class__.__qualname__, f"QC IFU DISTORT NSPOTS = {n_spots}")
        Msg.info(self.__class__.__qualname__, f"QC IFU DISTORT RMS = {rms}")
        Msg.info(self.__class__.__qualname__, f"QC IFU DISTORT FWHM = {fwhm}")

        if rms is None or fwhm is None:
            Msg.warning(self.__class__.__qualname__,
                        "No traces were detected on any detector, so the distortion "
                        "tables are empty and QC RMS/FWHM are not reported")

        return self.collect_qc_parameters(*qc)


class MetisIfuDistortion(Recipe):
    _name = "metis_ifu_distortion"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Determine the geometric distortion of the IFU."
    _description = (
        "Locates the spatial slices of the IFU image slicer on each of the four "
        "detectors and describes each one by a polynomial, producing a table of "
        "distortion coefficients that maps detector position to position on sky.\n"
        "Slit tilt is not determined yet; that requires a line-rich calibration "
        "frame, which this recipe does not receive.\n"
        "\n"
        "KNOWN DEVIATIONS FROM THE DRLD\n"
        "1. Input list. The DRLD gives this recipe only the multi-pinhole exposure, "
        "but the algorithm it prescribes locates slices by thresholding a "
        "continuum-illuminated frame, which a pinhole exposure is not: its features "
        "are isolated spots a few pixels across, not slice-long bands. An "
        "IFU_RSRF_RAW frame is therefore accepted as an optional additional input and "
        "traced in preference to the pinhole exposure. This is a deliberate deviation, "
        "agreed in review (PR #220) as the DRLD input list being at fault, and it is "
        "what makes the measured slice edges below possible at all. Without such a "
        "frame the recipe behaves exactly as the DRLD describes.\n"
        "2. Additional table columns. The distortion table carries `bottom` and `top` "
        "edge polynomials, and a `has_edges` flag, beyond the mid-line and column "
        "range the DRLD specifies. They record the measured illuminated extent of each "
        "slice so that metis_ifu_wavecal need not guess it back from the slice "
        "spacing. Readers that do not know the columns are unaffected, and tables "
        "written without them are still read.\n"
        "\n"
        "QC IFU DISTORT NSPOTS and FWHM are always measured on the pinhole exposure, "
        "as the DRLD defines them, even when the solution came from a continuum frame: "
        "a continuum frame shows slices rather than spots, so counting its features "
        "would report a slice count under a keyword defined as a number of spots."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Stack the raw pinhole exposures.
    Estimate the local background by smoothing along the cross-dispersion direction
    and threshold against it, to separate illuminated from inter-slice pixels.
    Discard the detector borders, then close gaps and remove specks morphologically.
    Label connected components and discard those too small or too narrow to constrain
    a fit, then merge the clusters that belong to the same slice.
    Fit a polynomial mid-line to each surviving cluster and tabulate its coefficients
    together with the valid column range.

    Adapted from PyReduce (Piskunov & Valenti 2002, Piskunov, Wehrhahn & Marquart
    2021), as prescribed by the DRLD for IFU distortion correction."""

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    # Tracing defaults are the METIS IFU values tuned in PyReduce.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
        ParameterRange(
            name=f"{_name}.trace.degree",
            context=_name,
            description="Polynomial degree of the fit to each slice",
            default=2,
            min=1,
            max=5,
        ),
        ParameterValue(
            name=f"{_name}.trace.degree_before_merge",
            context=_name,
            description="Polynomial degree used while rating candidate merges, where "
                        "the fits are still poorly constrained. An integer, or 'best' "
                        "to choose one per cluster",
            default="best",
        ),
        ParameterValue(
            name=f"{_name}.trace.min_cluster",
            context=_name,
            description="Smallest acceptable cluster of illuminated pixels, in pixels",
            default=1000,
        ),
        ParameterRange(
            name=f"{_name}.trace.min_width",
            context=_name,
            description="Smallest acceptable cluster extent along the dispersion "
                        "direction, as a fraction of the detector width. "
                        "0 disables the check",
            default=0.25,
            min=0.0,
            max=1.0,
        ),
        ParameterValue(
            name=f"{_name}.trace.filter_x",
            context=_name,
            description="Smoothing width along the dispersion direction, applied "
                        "before thresholding. 0 disables it",
            default=0,
        ),
        ParameterValue(
            name=f"{_name}.trace.filter_y",
            context=_name,
            description="Smoothing width along the cross-dispersion direction, "
                        "used to estimate the local background",
            default=200,
        ),
        ParameterEnum(
            name=f"{_name}.trace.filter_type",
            context=_name,
            description="Smoothing kernel used to estimate the local background. "
                        "'whittaker' preserves edges best, 'boxcar' is cheapest",
            default="boxcar",
            alternatives=("boxcar", "gaussian", "whittaker"),
        ),
        ParameterValue(
            name=f"{_name}.trace.noise",
            context=_name,
            description="Absolute detection threshold above the local background",
            default=120.0,
        ),
        ParameterValue(
            name=f"{_name}.trace.noise_relative",
            context=_name,
            description="Detection threshold as a fraction of the local background. "
                        "If this and the absolute threshold are both 0, 0.1% is used",
            default=0.0,
        ),
        ParameterValue(
            name=f"{_name}.trace.border_width",
            context=_name,
            description="Number of pixels to ignore at each detector edge",
            default=6,
        ),
        ParameterRange(
            name=f"{_name}.trace.auto_merge_threshold",
            context=_name,
            description="Overlap rating at or above which two clusters are merged "
                        "into one slice. 1 disables merging",
            default=0.9,
            min=0.0,
            max=1.0,
        ),
        ParameterRange(
            name=f"{_name}.trace.merge_min_threshold",
            context=_name,
            description="Overlap rating below which a pair of clusters is not "
                        "considered for merging",
            default=0.01,
            min=0.0,
            max=1.0,
        ),
        ParameterValue(
            name=f"{_name}.trace.sigma",
            context=_name,
            description="If positive, split clusters that deviate from the common "
                        "trace shape by more than this many standard deviations",
            default=0.0,
        ),
    ])


    Impl = MetisIfuDistortionImpl
