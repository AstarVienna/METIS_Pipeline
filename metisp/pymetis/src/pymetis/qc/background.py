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

from pymetis.classes.qc.parameter import QcParameter


class QcLmImgBkgMedian(QcParameter):
    _name_template = "QC LM IMG BKG MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "counts"
    _description_template = "Median value of the background removed LM image"


class QcLmImgBkgMedianDeviation(QcParameter):
    _name_template = "QC LM IMG BKG MEDIAN DEVIATION"
    _type = cpl.core.Type.DOUBLE
    _unit = "counts"
    _description_template = "Median deviation of the background removed LM image"
