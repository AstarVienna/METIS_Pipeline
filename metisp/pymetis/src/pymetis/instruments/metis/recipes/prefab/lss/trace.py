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

from typing import Literal

import cpl
from cpl.core import Msg

from pymetis.drl.combine import combine_images
from pymetis.drl.trace import measure_trace_edges, trace, traces_to_table

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import SinglePipelineInput
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_table

from pymetis.instruments.metis.dataitems.lss.rsrf import LssRsrfPinholeRaw
from pymetis.instruments.metis.dataitems.lss.trace import LssTrace, LssTraceImage
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
        TraceImage = LssTraceImage

    class Qc(QcParameterSet):
        LPolyDeg = QcLssTraceLPolyDeg
        RPolyDeg = QcLssTraceRPolyDeg
        LCoeff = QcLssTraceLCoeff
        RCoeff = QcLssTraceRCoeff
        InterorderLevel = QcLssTraceInterorderLevel

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

    def _collect_qc(self, table, degree) -> None:
        """
        # TODO - update this docstring
        Summarise the per-detector tracing results into quality control parameters.

        `NTRACES` counts the slices traced across the whole detector array. It is the
        only tracing-derived quantity now that tracing runs solely on
        continuum-illuminated frames, and there is no separate pinhole exposure to
        report a spot count or width from.
        """
        out = cpl.core.PropertyList()

        out.append(self.collect_qc_parameters(self.Qc.RPolyDeg(degree)))
        out.append(self.collect_qc_parameters(self.Qc.LPolyDeg(degree)))

        r_index = table.get_column_maxpos('trace_nb')
        l_index = table.get_column_minpos('trace_nb')

    
        # TODO - check if there is a cpl function for this, atm seems dodge
        # TODO - it is verified that the degree is the same len as pos + 1 in traces_to_table
        # should I be checking here too?
        for order, value in enumerate(table.column_array('pos')[0][r_index]):
            rcoeff_cls = self.Qc._specialized_item('RCoeff', self.Qc.RCoeff, order=order)
            out.append(self.collect_qc_parameters(rcoeff_cls(value)))

        for order, value in enumerate(table.column_array('pos')[0][l_index]):
            lcoeff_cls = self.Qc._specialized_item('LCoeff', self.Qc.LCoeff, order=order)
            out.append(self.collect_qc_parameters(lcoeff_cls(value)))

        # TODO check if there is an existing function to do this
        #self.collect_qc_parameters(self.Qc.InterorderLevel(trace_parameters))
        return out

    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""

        # TODO - I think this is 1 for LM and 2 for N, probably a proper way to
        # figure out which number is correct
        # TODO double check I don't need to do the same continuum filtering as in the IFU
        # TODO - no WCU grid mask in N band?
        det = 1
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')

        method = self.parameters[f"{self.name}.stacking.method"].value
        combined_image = combine_images(raw_images, method)

        trace_image = combined_image.as_array()
        # TODO - is this the correct approach?
        trace_image = trace_image.T

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

        qc_params = self._collect_qc(table, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))
        header_table.append(qc_params)

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        output = {
            'TABLE': Hdu(header_table, table, name=rf'TABLE'),
            'IMAGE': Hdu(header_image, combined_image, name='DET1.DATA'),
        }

        product_trace_table = self.ProductSet.TraceTable(
            header_table,
            output['TABLE'],
        )

        # TODO - are we saving the combined image?
        product_master_trace_img = self.ProductSet.TraceImage(
            header_image,
            output['IMAGE'],
        )

        return {product_trace_table, product_master_trace_img}

