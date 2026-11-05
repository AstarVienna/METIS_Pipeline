"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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

import cpl
from cpl.core import Msg

from pymetis.drl.combine import combine_images
from pymetis.drl.trace import measure_trace_edges, trace, traces_to_table

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import SinglePipelineInput
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_table

from pymetis.instruments.metis.dataitems.lss.rsrf import LssRsrfPinholeRaw, MasterLssRsrf
from pymetis.instruments.metis.dataitems.lss.trace import LssTrace
from pymetis.instruments.metis.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.instruments.metis.inputs import (RawInput, PersistenceMapInput, OptionalInputMixin,
                                              GainMapInput, LinearityInput, BadPixMapInput, MasterDarkInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import DarkImageProcessor
from pymetis.instruments.metis.qc.trace import (QcLssTraceLPolyDeg, QcLssTraceRPolyDeg,
                                                QcLssTraceLCoeff, QcLssTraceRCoeff, QcLssTraceInterorderLevel)


class MetisLssTraceImpl(DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        # TODO - ask about this  - thought this would be a flat continuum
        class RawInput(RawInput):
            Item = LssRsrfPinholeRaw

        class LmRsrfWcuOffInput(RawInput):
            Item = WcuOffRaw

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            pass

        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class MasterDarkInput(OptionalInputMixin, MasterDarkInput):
            pass

        raw: RawInput
        lm_rsrf_wcu_off: LmRsrfWcuOffInput
        bad_pix_map: BadPixMapInput
        linearity: LinearityInput
        gain_map: GainMapInput
        persistence_map: PersistenceMapInput
        master_dark: MasterDarkInput


    class ProductSet(PipelineProductSet):
        TraceTable = LssTrace

    class Qc(QcParameterSet):
        LPolyDeg = QcLssTraceLPolyDeg
        RPolyDeg = QcLssTraceRPolyDeg
        LCoeff = QcLssTraceLCoeff
        RCoeff = QcLssTraceRCoeff
        InterorderLevel = QcLssTraceInterorderLevel

    # TODO - copied directly from IFU, need to check if anything needs changing
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

    # TODO - this is copied over from IFU, need to change it to LSS ones
    def _collect_qc(self, table, degree) -> cpl.core.PropertyList:
        """
        Summarise the per-detector tracing results into quality control parameters.

        `NTRACES` counts the slices traced across the whole detector array. It is the
        only tracing-derived quantity now that tracing runs solely on
        continuum-illuminated frames, and there is no separate pinhole exposure to
        report a spot count or width from.
        """
        # TODO check the cpl way of doing this
        row_r = table['trace_nb'].idxmax()
        row_l = table['trace_nb'].idxmin()

        # TODO find the proper way of doing this
        band = 'LM'
        
        # TODO - it is verified that the degree is the same len as pos + 1 in traces_to_table
        # should I be checking here too?
        self.collect_qc_parameters(self.Qc.LPolyDeg(degree))
        Msg.info(self.__class__.__qualname__, f"QC {band} LSS TRACE LPOLYDEG = {degree}")
        self.collect_qc_parameters(self.Qc.RPolyDeg(degree))
        Msg.info(self.__class__.__qualname__, f"QC {band} LSS TRACE RPOLYDEG = {degree}")
        self.collect_qc_parameters(self.Qc.LCoeff(row_l['pos'][degree]))
        Msg.info(self.__class__.__qualname__, f"QC {band} LSS TRACE LCOEFForder = {row_l['pos'][degree]}")
        self.collect_qc_parameters(self.Qc.RCoeff(row_r['pos'][degree]))
        Msg.info(self.__class__.__qualname__, f"QC {band} LSS TRACE RCOEFForder = {row_r['pos'][degree]}")
        # TODO check if there is an existing function to do this
        #self.collect_qc_parameters(self.Qc.InterorderLevel(trace_parameters))


    # TODO - this is copied over from IFU, need to change it to LSS ones
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

    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""

        # TODO - I think this is 1 for LM and 2 for N, probably a proper way to
        # figure out which number is correct
        # TODO double check I don't need to do the same continuum filtering as in the IFU
        det = 1
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')

        method = self.parameters[f"{self.name}.stacking.method"].value
        combined_image = combine_images(raw_images, method)

        trace_image = combined_image.as_array()

        trace_parameters = self._trace_parameters()

        # CPL and HDRL have no order tracing, so the fitting is done in numpy
        traces = trace(trace_image, **trace_parameters)

        if not traces:
            raise cpl.core.DataNotFoundError(
                f"No traces found, the distortion table "
                f"would be empty and no downstream recipe could use it. Either the raw "
                f"frames carry no usable illumination, or the tracing parameters do not fit "
                f"them: check {self.name}.trace.noise (the absolute threshold above the "
                f"local background) and {self.name}.trace.min_cluster against the actual "
                f"signal level and the detector size."
            )

        # Measure the illuminated extent of the traces and augment the trace objects
        # with the edge polynomials.
        measure_trace_edges(trace_image, traces, degree=trace_parameters['degree'])

        table = traces_to_table(traces, trace_parameters['degree'])

        self._collect_qc(traces, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        output = [{
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
            'n_traces': len(traces),
        }]

        # TODO - might want to change this varibale name so it doesnt get confusing
        header_table = create_dummy_header()
        header_table.append(self._collect_qc(output))
        self._verify_any_trace_found(output)

        product_trace_table = self.ProductSet.TraceTable(
            header_table,
            *[out['TABLE'] for out in output],
        )

        return {product_trace_table}

