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
from pymetis.drl.trace import measure_trace_fwhm, trace, traces_to_table

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

    def _trace_parameters(self) -> dict:
        """Collect the tracing parameters from the recipe parameter list."""
        name = self.name
        return {
            'degree': int(self.parameters[f"{name}.trace.degree"].value),
            'min_cluster': int(self.parameters[f"{name}.trace.min_cluster"].value),
            'filter_y': int(self.parameters[f"{name}.trace.filter_y"].value),
            'noise': float(self.parameters[f"{name}.trace.noise"].value),
            'border_width': int(self.parameters[f"{name}.trace.border_width"].value),
            'auto_merge_threshold':
                float(self.parameters[f"{name}.trace.auto_merge_threshold"].value),
            'merge_min_threshold':
                float(self.parameters[f"{name}.trace.merge_min_threshold"].value),
        }

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
        if reference.frameset:
            reference_images = reference.use().load_data(extension=rf'DET{det}.DATA')
            trace_image = combine_images(reference_images, method).as_array()
            Msg.info(self.__class__.__qualname__,
                     f"DET{det}: tracing the {reference.Item.name()} frames rather than "
                     f"the pinhole exposure")
        else:
            trace_image = combined_image.as_array()

        # CPL and HDRL have no order tracing, so the fitting is done in numpy
        traces = trace(trace_image, **trace_parameters)

        if not traces:
            Msg.warning(self.__class__.__qualname__,
                        f"No traces detected on DET{det}; "
                        f"its distortion table will be empty")

        table = traces_to_table(traces, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        return {
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
            'residuals': [t.residual for t in traces if t.residual is not None],
            'n_traces': len(traces),
            'fwhm': measure_trace_fwhm(trace_image, traces),
        }

    def process(self) -> set[DataItem]:
        method = self.parameters[f"{self.name}.stacking.method"].value
        trace_parameters = self._trace_parameters()

        output = [self._process_single_detector(det, method, trace_parameters)
                  for det in [1, 2, 3, 4]]

        header_table = create_dummy_header()
        header_table.append(self._collect_qc(output))
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

    def _collect_qc(self, output: list[dict]) -> cpl.core.PropertyList:
        """
        Summarise the per-detector tracing results into quality control parameters.

        `NSPOTS` counts the traces found across the whole detector array. `RMS` pools
        the per-trace deviations of the measured mid-line from the fitted one. `FWHM`
        is the median cross-dispersion width of the traces, which indicates how the
        spectral resolution varies across the field of view.

        `RMS` and `FWHM` are undefined when no trace was detected. Their keywords are
        then left out of the header altogether, rather than filled with a placeholder
        that a consumer could mistake for a measurement.
        """
        n_traces = sum(out['n_traces'] for out in output)

        residuals = [r for out in output for r in out['residuals']]
        rms = float(np.sqrt(np.mean(np.square(residuals)))) if residuals else None

        fwhms = [out['fwhm'] for out in output if out['fwhm'] is not None]
        fwhm = float(np.median(fwhms)) if fwhms else None

        qc = [self.Qc.NSpots(n_traces)]
        if rms is not None:
            qc.append(self.Qc.Rms(rms))
        if fwhm is not None:
            qc.append(self.Qc.Fwhm(fwhm))

        Msg.info(self.__class__.__qualname__, f"QC IFU DISTORT NSPOTS = {n_traces}")
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
        "frame, which this recipe does not receive."
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
            name=f"{_name}.trace.min_cluster",
            context=_name,
            description="Smallest acceptable cluster of illuminated pixels, in pixels",
            default=1000,
        ),
        ParameterValue(
            name=f"{_name}.trace.filter_y",
            context=_name,
            description="Smoothing width along the cross-dispersion direction, "
                        "used to estimate the local background",
            default=200,
        ),
        ParameterValue(
            name=f"{_name}.trace.noise",
            context=_name,
            description="Absolute detection threshold above the local background",
            default=120.0,
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
    ])


    Impl = MetisIfuDistortionImpl
