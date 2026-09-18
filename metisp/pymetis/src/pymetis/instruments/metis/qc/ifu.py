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

QC parameters of the IFU recipes (calibrate, telluric, wavecal, postprocess).
"""

from pymetis.engine.qc import QcParameter


class IfuCalibMinFlux(QcParameter):
    _name_template = "QC IFU CALIB MINFLUX"
    _type = float
    _unit = "Jansky"
    _description_template = "Minimum pixel flux in the calibrated image"
    _comment = None


class IfuCalibMaxFlux(QcParameter):
    _name_template = "QC IFU CALIB MAXFLUX"
    _type = float
    _unit = "Jansky"
    _description_template = "Maximum pixel flux in the calibrated image"
    _comment = None


class IfuTelluricChi2(QcParameter):
    _name_template = "QC IFU TELLURIC CHI2"
    _type = float
    _unit = None
    _description_template = "Chi-squared of telluric fit from molecfit"


class IfuTelluricNpThreshold(QcParameter):
    _name_template = "QC IFU TELLURIC NPTHRESH"
    _type = float
    _unit = "counts"
    _description_template = "Number of pixels above the threshold used to calculate the spectrum"


class IfuTelluricConversion(QcParameter):
    _name_template = "QC IFU TELLURIC CONV"
    _type = float
    _unit = "Jansky / counts"
    _description_template = "Calculated conversion factor"


class IfuWavecalNLines(QcParameter):
    _name_template = "QC IFU WAVECAL NLINES"
    _type = int
    _unit = "counts"
    _default = None
    _description_template = "Number of detected laser lines; should be constant"


class IfuWavecalRms(QcParameter):
    _name_template = "QC IFU WAVECAL RMS"
    _type = float
    _unit = "Å"
    _default = None
    _description_template = "Root mean square of the residuals of the wavelength calibration fit"


class IfuWavecalPeakCounts(QcParameter):
    _name_template = "QC IFU WAVECAL PEAK CNTS"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Peak counts of the laser line"


class IfuWavecalLineWidth(QcParameter):
    _name_template = "QC IFU WAVECAL LINE WIDTH"
    _type = float
    _unit = "pixels"
    _default = None
    _description_template = "FWHM of the laser line as measured by fitting a Gaussian profile to it"
    _comment = "This fulfils METIS-6073"


class IfuPostprocGridRange(QcParameter):
    _name_template = "QC IFU POSTPROC GRIDRNG"
    _type = float
    _unit = "pixels"
    _description_template = "Maximum - minimum values of the interpolated grids"
    _comment = None


class IfuPostprocMedMean(QcParameter):
    _name_template = "QC IFU POSTPROC MEDMEAN"
    _type = float
    _unit = "Jansky"
    _description_template = "Mean of medians of regridded images"
    _comment = None


class IfuPostprocMedRms(QcParameter):
    _name_template = "QC IFU POSTPROC MEDRMS"
    _type = float
    _unit = "Jansky"
    _description_template = "Root-mean-square of the medians of the regridded images"
    _comment = None


class IfuPostprocMedMed(QcParameter):
    _name_template = "QC IFU POSTPROC MEDMED"
    _type = float
    _unit = "Jansky"
    _description_template = "Median of the medians of the regridded images"
    _comment = None


class IfuPostprocDeltaC(QcParameter):
    _name_template = "QC IFU POSTPROC DELTAC"
    _type = float
    _unit = "pixels"
    _default = None
    _description_template = "Range of shifts in the center position for regridding"
