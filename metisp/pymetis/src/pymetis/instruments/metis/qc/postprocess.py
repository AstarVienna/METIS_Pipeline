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

QC parameters of the LM imaging science post-processing.
"""

from pymetis.engine.qc import QcParameter


class LmSciNExp(QcParameter):
    _name_template = "QC LM SCI NEXP"
    _type = int
    _unit = "counts"
    _default = None
    _description_template = "Number of images that went into a LM_SCI_COADD"


class LmSciPostprocGridRange(QcParameter):
    _name_template = "QC LM SCI POSTPROC GRIDRNG"
    _type = float
    _unit = "pixels"
    _default = None
    _description_template = "Maximum - minimum values of the interpolated grids"


class LmSciPostprocMedMean(QcParameter):
    _name_template = "QC LM SCI POSTPROC MEDMEAN"
    _type = float
    _unit = "Jansky"
    _default = None
    _description_template = "Mean of the medians of the regridded images"


class LmSciPostprocMedRms(QcParameter):
    _name_template = "QC LM SCI POSTPROC MEDRMS"
    _type = float
    _unit = "Jansky"
    _default = None
    _description_template = "Root-mean-squared of the medians of the regridded images"


class LmSciPostprocMedMed(QcParameter):
    _name_template = "QC LM SCI POSTPROC MEDMED"
    _type = float
    _unit = "Jansky"
    _default = None
    _description_template = "Median of the medians of the regridded images"


class LmSciPostprocDeltaCentre(QcParameter):
    _name_template = "QC LM SCI POSTPROC DELTAC"
    _type = float
    _unit = "pixels"
    _default = None
    _description_template = "Range of shifts in the center position for regridding"
