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

from pymetis.classes.qc import QcParameter


class QcLssTraceLPolyDeg(QcParameter):
    _name_template = "QC {band} LSS TRACE LPOLYDEG"
    _type = int
    _unit = "1"
    _description_template = "Degree of the polynomial fit of the left order edge"
    _comment = '"Left" edge means the order edge closer to the left detector edge'


class QcLssTraceLCoeff(QcParameter):
    _name_template = "QC {band} LSS TRACE LCOEFF{order}"
    _type = float
    _unit = "pixels^(1 - i)"
    _description_template = "i-th coefficient of the polynomial of the left order edge"
    _comment = '"Left" edge means the order edge closer to the left detector edge'


class QcLssTraceRPolyDeg(QcParameter):
    _name_template = "QC {band} LSS TRACE RPOLYDEG"
    _type = int
    _unit = "1"
    _description_template = "Degree of the polynomial fit of the right order edge"
    _comment = '"Right" edge means the order edge closer to the right detector edge'


class QcLssTraceRCoeff(QcParameter):
    _name_template = "QC {band} LSS TRACE RCOEFF{order}"
    _type = float
    _unit = "pixels^(1 - i)"
    _description_template = "i-th coefficient of the polynomial of the right order edge"
    _comment = '"Right" edge means the order edge closer to the right detector edge'


class QcLssTraceInterorderLevel(QcParameter):
    _name_template = "QC {band} LSS TRACE INTORDR LEVEL"
    _type = float
    _unit = 'counts'
    _description_template = "Flux level of the interorder background"
    _comment = "Determined outside the order"