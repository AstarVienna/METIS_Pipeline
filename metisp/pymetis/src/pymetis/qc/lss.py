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


class LssSnr(QcParameter):
    _name_template = "QC {band} LSS {target} SNR"
    _type = float
    _unit = "1"
    _description_template = "Signal-to-noise ratio of science spectrum"
    _comment = None


class LssNoiseLevel(QcParameter):
    _name_template = "QC {band} LSS {target} NOISELEV"
    _type = float
    _unit = "Jansky"
    _description_template = "Noise level of science spectrum"
    _comment = None


class LssInterorderLevel(QcParameter):
    _name_template = "QC {band} LSS {target} INTORDR LEVEL"
    _type = float
    _unit = "Jansky"
    _description_template = "Flux level of the interorder background"
    _comment = None


class LssWaveCalDevMean(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL DEVMEAN"
    _type = float
    _unit = "Å"
    _description_template = "Mean deviation from the wavelength reference frame"
    _comment = None


class LssWaveCalFwhm(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL FWHM"
    _type = float
    _unit = "Å"
    _description_template = "Measured FWHM of lines"
    _comment = None


class LssWaveCalNIdent(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL NIDENT"
    _type = int
    _unit = "1"
    _description_template = "Number of identified lines"
    _comment = None


class LssWaveCalNMatch(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL NMATCH"
    _type = int
    _unit = "1"
    _description_template = "Number of line matched between catalogue and spectrum"
    _comment = None


class LssWaveCalPolyDeg(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL POLYDEG"
    _type = int
    _unit = "1"
    _description_template = "Degree of the wavelength polynomial"
    _comment = None


class LssWaveCalPolyCoeffN(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL POLYCOEFF{n}"
    _type = float
    _unit = "Å/pixel^(n + 1)"
    _description_template = "{n}-th coefficient of the wavelength polynomial"