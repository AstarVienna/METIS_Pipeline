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

import cpl

from pymetis.classes.dataitems import TableDataItem
from pymetis.classes.mixins import BandSpecificMixin, BandLmMixin, BandNMixin, BandIfuMixin


class DistortionTable(BandSpecificMixin, TableDataItem, abstract=True):
    _name_template = r'{band}_DISTORTION_TABLE'
    _title_template = "distortion table"
    _description_template = r"Table of distortion coefficients for a {band} band data set"
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class LmDistortionTable(BandLmMixin, DistortionTable):
    pass


class NDistortionTable(BandNMixin, DistortionTable):
    pass


class IfuDistortionTable(BandIfuMixin, DistortionTable):
    def read(self, *, extension: int = 1) -> cpl.core.ImageList:
        # Load the distortion table
        # TODO: assumes distortion table has one set of coefficients for each extension
        distortion_table = cpl.core.Table.load(self.frame.file, extension=extension)

        # obtain the trace polynomials from the distortion table
        trace_polys = distortion_table.column_array('orders')[0]
        x_ranges = distortion_table.column_array('column_range')[0]

        # create a list of y-coordinates for each trace from the distortion table
        # x_arr = np.arange(0, rsrf_raw_img.width)
        trace_list = []
        for x_range, trace in zip(x_ranges, trace_polys):
            x_arr = np.arange(x_range[0], x_range[1])
            poly_n = len(trace) - 1
            y_arr = np.array([sum([k * x ** (poly_n - i) for i, k in enumerate(trace)]) for x in x_arr])
            trace_list.append((x_arr, y_arr))

        # return the list of x,y coordinates for each trace
        return trace_list

