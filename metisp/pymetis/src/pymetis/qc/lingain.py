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


class LinGainMean(QcParameter):
    _name_template = "QC LIN GAIN MEAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Mean value of the gain"


class LinGainRms(QcParameter):
    _name_template = "QC LIN GAIN RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Root mean square of the gain values"


class LinMinFlux(QcParameter):
    _name_template = "QC LIN MIN FLUX"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Minimum flux in images"


class LinMaxFlux(QcParameter):
    _name_template = "QC LIN MAX FLUX"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Maximum flux in images"


class LinNumBadpix(QcParameter):
    _name_template = "QC LIN NUM BADPIX"
    _type = cpl.core.Type.INT
    _unit = "counts"
    _description_template = "Number of bad pixels"


class GainLin(QcParameter):
    _name_template = "QC GAIN LIN"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Effective linearity"


class GainCoeff(QcParameter):
    _name_template = "QC CAL CHOPHOME SNR"
    _type = cpl.core.Type.DOUBLE
    _unit = "e / adu"
    _description_template = "Linearity coefficients"

