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


class DarkMean(QcParameter):
    _name = "QC DARK MEAN"
    _type = cpl.core.Type.DOUBLE
    _description = "Mean level of the dark frame"


class DarkMedian(QcParameter):
    _name = "QC DARK MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _description = "Median level of the dark frame"


class DarkRms(QcParameter):
    _name = "QC DARK RMS"
    _type = cpl.core.Type.DOUBLE
    _description = "RMS level of the dark frame"


class DarkNBadpix(QcParameter):
    _name = "QC DARK NBADPIX"
    _type = cpl.core.Type.INT
    _description = "Number of bad pixels in the image mask"


class DarkNColdpix(QcParameter):
    _name = "QC DARK NCOLDPIX"
    _type = cpl.core.Type.INT
    _description = "Number of cold pixels in the image mask"


class DarkNHotpix(QcParameter):
    _name = "QC DARK NHOTPIX"
    _type = cpl.core.Type.INT
    _description = "Number of cold pixels in the image mask"


class DarkMedianMedian(QcParameter):
    _name = "QC DARK MEDIAN MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _description = "Median of the median values of individual dark frames"


class DarkMedianMean(QcParameter):
    _name = "QC DARK MEDIAN MEAN"
    _type = cpl.core.Type.DOUBLE
    _description = "Mean of the median values of individual dark frames"


class DarkMedianRms(QcParameter):
    _name = "QC DARK MEDIAN RMS"
    _type = cpl.core.Type.DOUBLE
    _description = "RMS of the median values of individual dark frames"


class DarkMedianMin(QcParameter):
    _name = "QC DARK MEDIAN MIN"
    _type = cpl.core.Type.DOUBLE
    _description = "Minimum of the median values of individual dark frames"


class DarkMedianMax(QcParameter):
    _name = "QC DARK MEDIAN MAX"
    _type = cpl.core.Type.DOUBLE
    _description = "Maximum of the median values of individual dark frames"