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

from pymetis.engine.core.parameter import (ParameterList, ParameterEnum, ParameterRange,
                                           ParameterValue)
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet, QcParameter
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.drl.combine import combine_images
from pymetis.drl.trace import measure_trace_edges, trace, traces_to_table

from pymetis.instruments.metis.inputs import (RawInput, MasterDarkInput, OptionalInputMixin, PersistenceMapInput,
                                              GainMapInput, LinearityInput)
from pymetis.instruments.metis.mixins import DetectorIfuMixin
from pymetis.instruments.metis.dataitems.distortion import IfuDistortionRaw, IfuDistortionTable, IfuDistortionReduced
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab.darkimage import DarkImageProcessor

class MetisIfuDistortionImpl(DetectorIfuMixin, DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class MasterDarkInput(OptionalInputMixin, MasterDarkInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class RawInput(RawInput):
            Item = IfuDistortionRaw

    class ProductSet(PipelineProductSet):
        DistortionTable = IfuDistortionTable
        DistortionReduced = IfuDistortionReduced

    class Qc(QcParameterSet):
        class NTraces(QcParameter):
            _name_template = "QC IFU DISTORT NTRACES"
            _type = int
            _unit = "1"
            _default = None
            _description_template = "Number of slices traced"

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

    def _continuum_frames(self, raw) -> list[int]:
        """
        Indices of the raw frames that are continuum-illuminated.

        The raw input can contain a mixture of exposures: some taken with the WCU
        focal-plane (pinhole) mask out of the beam, illuminating the slices with a
        continuum, and some taken with it in the beam, illuminating them with a grid of
        pinhole spots. Nothing about the frame's tag tells the two apart -- both are
        `IFU_DISTORTION_RAW` -- only the WCU mask position keyword,
        `ESO INS OPTI20 POSNAME` (`open` versus a grid position), does. Tracing a
        pinhole-illuminated frame here would defeat the purpose, since its
        cross-dispersion profile has the width of a spot rather than of a slice, so
        pinhole-illuminated frames are dropped rather than averaged in.

        Returns
        -------
        list[int]
            Positions within `raw.frameset` that are continuum-illuminated, empty if
            none qualify.
        """
        if not raw.frameset:
            return []

        continuum = []
        for index, frame in enumerate(raw.frameset):
            header = cpl.core.PropertyList.load(frame.file, 0)
            keyword = 'ESO INS OPTI20 POSNAME'

            if keyword not in header:
                Msg.warning(self.__class__.__qualname__,
                            f"{frame.file}: no {keyword}, so it cannot be told apart "
                            f"from a pinhole-grid exposure; not used for tracing")
                continue

            if str(header[keyword].value).strip() == self.OPEN_MASK:
                continuum.append(index)

        dropped = len(raw.frameset) - len(continuum)
        if dropped:
            Msg.info(self.__class__.__qualname__,
                     f"Ignoring {dropped} raw frame(s) taken through a focal-plane "
                     f"mask; only unmasked frames show whole slices")

        return continuum

    def _process_single_detector(self,
                                 detector: Literal[1, 2, 3, 4],
                                 method: str,
                                 trace_parameters: dict) -> dict:
        """
        Determine the geometric distortion for a single detector of the IFU.

        Only continuum-illuminated raw frames are traced: the spatial slices of the
        image slicer are located by thresholding against a smoothed local background
        and fitting a polynomial mid-line to each resulting cluster of illuminated
        pixels, which needs the slice-long bands of illumination that only a continuum
        frame provides. This is the algorithm the DRLD prescribes for IFU distortion
        (critical algorithm 5b).

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4] # FixMe: Maybe make this fully customizable for any detector count?
        method : str
            Method used to stack the continuum-illuminated exposures.
        trace_parameters : dict
            Keyword arguments for `pymetis.drl.trace.trace`.

        Returns
        -------
        dict
            The distortion table and stacked image HDUs, plus the quantities needed
            for the recipe's quality control parameters. `TABLE` and `IMAGE` are
            `None` when no continuum-illuminated frame was found.
        """
        det = rf'{detector:1d}'

        # `_continuum_frames` classifies frames by header content on the raw frameset,
        # which is the same for every detector, so this is either empty for all four
        # detectors or none of them -- a single detector coming back empty on its own
        # cannot happen.
        continuum = self._continuum_frames(self.inputset.raw)

        if not continuum:
            Msg.warning(self.__class__.__qualname__,
                        f"DET{det}: no continuum-illuminated raw frames found; "
                        f"its distortion table and reduced image will be empty")
            return {'TABLE': None, 'IMAGE': None, 'n_traces': 0}

        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')
        selected = cpl.core.ImageList()
        for index in continuum:
            selected.append(raw_images[index])

        combined_image = combine_images(selected, method)
        trace_image = combined_image.as_array()
        Msg.info(self.__class__.__qualname__,
                 f"DET{det}: tracing {len(continuum)} of "
                 f"{len(self.inputset.raw.frameset)} raw frame(s) taken without "
                 f"the pinhole mask")

        # CPL and HDRL have no order tracing, so the fitting is done in numpy
        traces = trace(trace_image, **trace_parameters)

        if not traces:
            Msg.warning(self.__class__.__qualname__,
                        f"No traces detected on DET{det}; "
                        f"its distortion table will be empty")

        # Measure the illuminated extent on the same frame the traces came from, so that
        # consumers read the aperture off the table instead of guessing it back from the
        # spacing between mid-lines.
        measure_trace_edges(trace_image, traces, degree=trace_parameters['degree'])

        table = traces_to_table(traces, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        return {
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
            'n_traces': len(traces),
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

        `NTRACES` counts the slices traced across the whole detector array. It is the
        only tracing-derived quantity now that tracing runs solely on
        continuum-illuminated frames, and there is no separate pinhole exposure to
        report a spot count or width from.
        """
        n_traces = sum(out['n_traces'] for out in output)

        Msg.info(self.__class__.__qualname__, f"QC IFU DISTORT NTRACES = {n_traces}")

        return self.collect_qc_parameters(self.Qc.NTraces(n_traces))


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
        "1. Tracing source. The DRLD gives this recipe only the multi-pinhole "
        "exposure, but the algorithm it prescribes locates slices by thresholding a "
        "continuum-illuminated frame, which a pinhole exposure is not: its features "
        "are isolated spots a few pixels across, not slice-long bands. Frames in the "
        "IFU_DISTORTION_RAW input taken with the WCU focal-plane mask out of the "
        "beam (ESO INS OPTI20 POSNAME = open) are therefore identified as "
        "continuum-illuminated and are the only frames traced; any pinhole-illuminated "
        "frames present in the same input are ignored for tracing purposes. This is a "
        "deliberate deviation, agreed in review (PR #220) as the DRLD input list being "
        "at fault, and it is what makes the measured slice edges below possible at "
        "all. When no continuum-illuminated frame is present, the recipe produces no "
        "output and fails, rather than falling back to tracing the pinhole exposure "
        "the DRLD prescribes.\n"
        "2. Additional table columns. The distortion table carries `bottom` and `top` "
        "edge polynomials, and a `has_edges` flag, beyond the mid-line and column "
        "range the DRLD specifies. They record the measured illuminated extent of each "
        "slice so that metis_ifu_wavecal need not guess it back from the slice "
        "spacing. Readers that do not know the columns are unaffected, and tables "
        "written without them are still read.\n"
        "\n"
        "QC IFU DISTORT NTRACES reports the total number of slices traced across all "
        "four detectors."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Stack the continuum-illuminated raw exposures.
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
