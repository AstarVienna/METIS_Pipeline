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
    _name_template = "QC DARK MEAN"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Mean level of the dark frame"


class DarkMedian(QcParameter):
    _name_template = "QC DARK MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Median level of the dark frame"


class DarkRms(QcParameter):
    _name_template = "QC DARK RMS"
    _type = cpl.core.Type.DOUBLE
    _description_template = "RMS level of the dark frame"


class DarkNBadpix(QcParameter):
    _name_template = "QC DARK NBADPIX"
    _type = cpl.core.Type.INT
    _description_template = "Number of bad pixels in the image mask"


class DarkNColdpix(QcParameter):
    _name_template = "QC DARK NCOLDPIX"
    _type = cpl.core.Type.INT
    _description_template = "Number of cold pixels in the image mask"


class DarkNHotpix(QcParameter):
    _name_template = "QC DARK NHOTPIX"
    _type = cpl.core.Type.INT
    _description_template = "Number of cold pixels in the image mask"


class DarkMedianMedian(QcParameter):
    _name_template = "QC DARK MEDIAN MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Median of the median values of individual dark frames"


class DarkMedianMean(QcParameter):
    _name_template = "QC DARK MEDIAN MEAN"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Mean of the median values of individual dark frames"


class DarkMedianRms(QcParameter):
    _name_template = "QC DARK MEDIAN RMS"
    _type = cpl.core.Type.DOUBLE
    _description_template = "RMS of the median values of individual dark frames"


class DarkMedianMin(QcParameter):
    _name_template = "QC DARK MEDIAN MIN"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Minimum of the median values of individual dark frames"


class DarkMedianMax(QcParameter):
    _name_template = "QC DARK MEDIAN MAX"
    _type = cpl.core.Type.DOUBLE
    _description_template = "Maximum of the median values of individual dark frames"