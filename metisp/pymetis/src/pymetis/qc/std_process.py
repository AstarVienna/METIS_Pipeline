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


class QcImgStdBackgroundRms(QcParameter):
    _name_template = "QC {band} IMG STD BACKGD RMS"
    _type = float
    _unit = "counts"
    _description_template = "RMS of the background of the standard star photometry"
    _comment = ""


class QcStdPeakCounts(QcParameter):
    _name_template = "QC {band} STD PEAK CNTS"
    _type = float
    _unit = "counts"
    _description_template = "Peak counts of the standard star"
    _comment = ""


class QcStdApertureCounts(QcParameter):
    _name_template = "QC {band} STD APERTURE CNTS"
    _type = float
    _unit = "counts"
    _description_template = "Aperture counts of the standard star"


class QcStdStrehl(QcParameter):
    _name_template = "QC {band} STD STREHL"
    _type = float
    _unit = "1"
    _description_template = "Strehl ratio of the standard star"


class QcStdAirmass(QcParameter):
    _name_template = "QC {band} STD FWHM"
    _type = float
    _unit = "1"
    _description_template = "Ellipticity of the standard star PSF"


class QcStdEllipticity(QcParameter):
    _name_template = "QC {band} STD FWHM"
    _type = float
    _unit = "1"
    _description_template = "Ellipticity of the standard star PSF"


class QcStdFluxConversion(QcParameter):
    _name_template = "QC {band} STD FLUXCONV"
    _type = float
    _unit = "Jansky / count"
    _description_template = "Flux conversion to physical units, determined from the standard star"


class QcSensitivity(QcParameter):
    _name_template = "QC {band} SENS"
    _type = float
    _unit = "Jansky"
    _description_template = "Sensitivity of point source detections"


class QcAreaSensitivity(QcParameter):
    _name_template = "QC {band} AREA SENS"
    _type = float
    _unit = "Jansky / pixel"
    _description_template = "Sensitivity of extended source detections"
