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

from pymetis.engine.qc import QcParameter


class LssSnr(QcParameter):
    _name_template = "QC {band} LSS {target} SNR"
    _type = float
    _unit = None
    _description_template = "Signal-to-noise ratio of the {target} spectrum"
    _comment = None


class LssNoiseLevel(QcParameter):
    _name_template = "QC {band} LSS {target} NOISELEV"
    _type = float
    _unit = "counts"
    _description_template = "Noise level of the {target} spectrum"
    _comment = None


class LssInterorderLevel(QcParameter):
    _name_template = "QC {band} LSS {target} INTORDR LEVEL"
    _type = float
    _unit = "counts"
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
    _unit = "counts"
    _description_template = "Number of identified lines"
    _comment = None


class LssWaveCalNMatch(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL NMATCH"
    _type = int
    _unit = "counts"
    _description_template = "Number of lines matched between catalogue and spectrum"
    _comment = None


class LssWaveCalPolyDeg(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL POLYDEG"
    _type = int
    _unit = None
    _description_template = "Degree of the wavelength polynomial"
    _comment = None


class LssWaveCalPolyCoeffN(QcParameter):
    _name_template = "QC {band} LSS {target} WAVECAL POLYCOEFF{n}"
    _type = float
    _unit = "Å/pixel^(n + 1)"
    _description_template = "{n}-th coefficient of the wavelength polynomial"


class LssStdPsfLoss(QcParameter):
    _name_template = "QC {band} LSS STD PSFLOSS"
    _type = float
    _unit = None
    _description_template = "Fraction of AO-induced slit losses of the standard star"


class LssRsrfMeanLevel(QcParameter):
    _name_template = "QC {band} LSS RSRF MEAN LEVEL"
    _type = float
    _unit = "counts"
    _description_template = "Mean level of the RSRF"


class LssRsrfMedianLevel(QcParameter):
    _name_template = "QC {band} LSS RSRF MEDIAN LEVEL"
    _type = float
    _unit = "counts"
    _description_template = "Median level of the RSRF"


class LssRsrfInterorderLevel(QcParameter):
    _name_template = "QC {band} LSS RSRF INTORDR LEVEL"
    _type = float
    _unit = "counts"
    _description_template = "Flux level of the interorder background"


class LssRsrfNormStdev(QcParameter):
    _name_template = "QC {band} LSS RSRF NORM STDEV"
    _type = float
    _unit = "counts"
    _description_template = "Standard deviation of the normalized RSRF"


class LssRsrfNormSnr(QcParameter):
    _name_template = "QC {band} LSS RSRF NORM SNR"
    _type = float
    _unit = None
    _description_template = "SNR of the normalized RSRF"


class LssStdBackgroundMean(QcParameter):
    _name_template = "QC {band} LSS STD BACKGD MEAN"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Mean value of background"


class LssStdBackgroundMedian(QcParameter):
    _name_template = "QC {band} LSS STD BACKGD MEDIAN"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Median value of background"


class LssStdBackgroundStdev(QcParameter):
    _name_template = "QC {band} LSS STD BACKGD STDEV"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Standard deviation value of background"
    _comment = None


class LssStdFwhm(QcParameter):
    _name_template = "QC {band} LSS STD FWHM"
    _type = float
    _unit = "Å"
    _default = None
    _description_template = "FWHM of flux standard spectrum"
    _comment = None


class LssStdAverageLevel(QcParameter):
    _name_template = "QC {band} LSS STD AVGLEVEL"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Average level of the standard star flux"
    _comment = None


class LssSciFluxSnr(QcParameter):
    _name_template = "QC {band} LSS SCI FLUX SNR"
    _type = float
    _unit = None
    _description_template = "Signal-to-noise ratio of flux calibrated science spectrum"
    _comment = None


class LssSciFluxNoiseLevel(QcParameter):
    _name_template = "QC {band} LSS SCI FLUX NOISELEV"
    _type = float
    _unit = "Jansky"
    _default = None
    _description_template = "Noise level of flux calibrated science spectrum"
    _comment = None


class LmLssWavePolyDeg(QcParameter):
    _name_template = "QC LM LSS WAVE POLYDEG"
    _type = int
    _unit = None
    _default = None
    _description_template = "Degree of the first guess polynomial"
    _comment = None


class LmLssWaveCoeffN(QcParameter):
    _name_template = "QC LM LSS WAVE COEFF{i}"
    _type = float
    _unit = "pixels^(1 - i)"
    _default = None
    _description_template = "{i}-th coefficient of the first guess polynomial"
    _comment = None


class LmLssWaveNLines(QcParameter):
    _name_template = "QC LM LSS WAVE NLINES"
    _type = int
    _unit = "counts"
    _default = None
    _description_template = "Number of detected laser lines; should be constant"


class LmLssWaveLineFwhmAvg(QcParameter):
    _name_template = "QC LM LSS WAVE LINEFWHMAVG"
    _type = float
    _unit = "Å"
    _default = None
    _description_template = "Average of the FWHM of the detected lines (should be widely constant)"
    _comment = None


class LmLssWaveInterorderLevel(QcParameter):
    _name_template = "QC LM LSS WAVE INTORDR LEVEL"
    _type = float
    _unit = "counts"
    _default = None
    _description_template = "Flux level of the interorder background"
    _comment = None
