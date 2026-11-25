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
from pymetis.drl.trace import (measure_trace_edges, trace, traces_to_table,
                               collect_trace_parameters)


from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.inputs import SinglePipelineInput
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header, create_dummy_table

from pymetis.instruments.metis.dataitems.lss.trace import LssTrace, LssTraceFlatImage, LssTracePinhImage
from pymetis.instruments.metis.inputs import (RawInput, PersistenceMapInput, OptionalInputMixin,
                                              GainMapInput, LinearityInput, BadPixMapInput, MasterDarkInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import DarkImageProcessor
from pymetis.instruments.metis import qc
from pymetis.instruments.metis import dataitems


class MetisLssTraceImpl(DarkImageProcessor, MetisRecipeImpl):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = dataitems.LssRsrfRaw

        class PinhRawInput(RawInput):
            Item = dataitems.LssRsrfPinholeRaw

        class LmRsrfWcuOffInput(RawInput):
            Item = dataitems.WcuOffRaw

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

        # TODO - not confident about approach to inputing the data
        raw: RawInput
        pinh_raw: PinhRawInput
        lm_rsrf_wcu_off: LmRsrfWcuOffInput
        bad_pix_map: BadPixMapInput
        linearity: LinearityInput
        gain_map: GainMapInput
        persistence_map: PersistenceMapInput
        master_dark: MasterDarkInput


    class ProductSet(PipelineProductSet):
        TraceTable = LssTrace
        FlatImage = LssTraceFlatImage
        PinhImage = LssTracePinhImage


    class Qc(QcParameterSet):
        LPolyDeg = qc.trace.QcLssTraceLPolyDeg
        RPolyDeg = qc.trace.QcLssTraceRPolyDeg
        LCoeff = qc.trace.QcLssTraceLCoeff
        RCoeff = qc.trace.QcLssTraceRCoeff
        InterorderLevel = qc.trace.QcLssTraceInterorderLevel
    

    def _collect_qc(self, table, degree) -> cpl.core.PropertyList:
        """
        Collect quality control parameters describing the fitted trace polynomials.

        Records the polynomial degree and the coefficients of the leftmost and
        rightmost traces on the detector. The leftmost trace (minimum ``trace_nb``)
        and rightmost trace (maximum ``trace_nb``).

        Parameters
        ----------
        table : cpl.core.Table
            Trace table as produced by ``traces_to_table``. Must contain columns
            ``trace_nb`` and ``pos``.
        degree : int
            Polynomial degree of the trace fits, used to populate ``LPolyDeg``
            and ``RPolyDeg``. The ``pos`` column is expected to hold ``degree + 1``
            coefficients per row, low-order first.

        Returns
        -------
        cpl.core.PropertyList
            A property list containing all collected QC parameters, suitable for
            appending to a product header.
        """
        out = cpl.core.PropertyList()

        # TODO - I'm taking this directly from the user input, is this alright?
        out.append(self.collect_qc_parameters(self.Qc.RPolyDeg(degree)))
        out.append(self.collect_qc_parameters(self.Qc.LPolyDeg(degree)))

        # TODO verify that this is indeed the correct right and left and this is the way to determine this
        r_index = table.get_column_maxpos('trace_nb')
        l_index = table.get_column_minpos('trace_nb')

        ## TODO - should double check that each order should be saved
        #for order, value in enumerate(table.column_array('pos')[0][r_index]):
        #    rcoeff_cls = self.Qc._specialized_item('RCoeff', self.Qc.RCoeff, order=order)
        #    out.append(self.collect_qc_parameters(rcoeff_cls(value)))

        #for order, value in enumerate(table.column_array('pos')[0][l_index]):
        #    lcoeff_cls = self.Qc._specialized_item('LCoeff', self.Qc.LCoeff, order=order)
        #    out.append(self.collect_qc_parameters(lcoeff_cls(value)))

        # TODO check if there is an existing function to do this
        # TODO - also not sure what this would be if this doesnt use pinh frames
        #self.collect_qc_parameters(self.Qc.InterorderLevel(trace_parameters))
        return out

    def _process_raw_frames(self, raw_images, stacking_method):

        combined_image = combine_images(raw_images, stacking_method)

        if stacking_method == 'sigclip':
            combined_image = combined_image[0]

        trace_image = combined_image.as_array()
        # TODO - is this the correct approach?
        trace_image = trace_image.T

        trace_parameters = collect_trace_parameters(self.name, self.parameters)

        # CPL and HDRL have no order tracing, so the fitting is done in numpy
        traces = trace(trace_image, **trace_parameters)

        # TODO think about this error more carefully
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
        # TODO - it cant measure the edges of the slit lumination
        measure_trace_edges(trace_image, traces, degree=trace_parameters['degree'])

        table = traces_to_table(traces, trace_parameters['degree'])

        return combined_image, table


    def process(self) -> set[DataItem]:
        """Create a dummy file (should do something more fancy in the future)"""
        # TODO - no WCU grid mask in N band for scopesim

        # TODO - is there a way to not hardcode this? makes me uncomfortable
        det = 1

        # Process the flat continuum images to find edges of slit
        flat_raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')
        flat_method = self.parameters[f"{self.name}.stacking_flat.method"].value

        flat_combined_image, edges_table = self._process_raw_frames(flat_raw_images, flat_method)

        # Process pinh images to find distortion shape
        pinhraw_images = self.inputset.pinh_raw.use().load_data(extension=rf'DET{det}.DATA')
        pinh_method = self.parameters[f"{self.name}.stacking_pinh.method"].value
        
        pinh_combined_image, pinh_table = self._process_raw_frames(pinhraw_images, pinh_method)

        # TODO - consider qc params properly
        #qc_params = self._collect_qc(table, trace_parameters['degree'])

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}'))
        #header_table.append(qc_params)

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        output = {
            'TABLE': Hdu(header_table, edges_table, name=rf'TABLE'),
            'FLAT_IMAGE': Hdu(header_image, flat_combined_image, name='DET1.DATA'),
            'PINH_IMAGE': Hdu(header_image, pinh_combined_image, name='DET1.DATA'),

        }

        product_trace_table = self.ProductSet.TraceTable(
            header_table,
            output['TABLE'],
        )

        product_master_flat_img = self.ProductSet.FlatImage(
            header_image,
            output['FLAT_IMAGE'],
        )

        product_master_pinh_img = self.ProductSet.PinhImage(
            header_image,
            output['PINH_IMAGE'],
        )

        # Only returning images if running in DEBUG mode
        if Msg.get_config()['level'] == Msg.SeverityLevel.DEBUG:
            return {product_trace_table, product_master_flat_img, product_master_pinh_img}
        else:
            return {product_trace_table}



