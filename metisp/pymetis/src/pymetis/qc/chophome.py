
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


class CalChophomeXcen(QcParameter):
    _name = "QC CAL CHOPHOME XCEN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Centroid of point source in x"


class CalChophomeXcenStdev(QcParameter):
    _name = "QC CAL CHOPHOME XCEN STDEV"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Uncertainty of centroid in x"


class CalChophomeYcen(QcParameter):
    _name = "QC CAL CHOPHOME YCEN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Centroid of point source in y"


class CalChophomeYcenStdev(QcParameter):
    _name = "QC CAL CHOPHOME YCEN STDEV"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Uncertainty of centroid in y"


class CalChophomeFwhm(QcParameter):
    _name = "QC CAL CHOPHOME FWHM"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "FWHM of point source"


class CalChophomeSnr(QcParameter):
    _name = "QC CAL CHOPHOME SNR"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Signal-to-noise ratio of point source"


class CalChophomeOffx(QcParameter):
    _name = "QC CAL CHOPHOME OFFX"
    _type = cpl.core.Type.DOUBLE
    _unit = "Pixel"
    _description = "Chopper offset in x"


class CalChophomeOffy(QcParameter):
    _name = "QC CAL CHOPHOME OFFY"
    _type = cpl.core.Type.DOUBLE
    _unit = "mas" # ToDo this is inconsistent (thanks ChatGPT)
    _description = "Chopper offset in y"
