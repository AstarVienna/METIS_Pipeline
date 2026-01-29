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
from pymetis.classes.mixins import BandLmMixin, BandNMixin
from pymetis.classes.qc import QcParameter


class QcDistortRms(QcParameter):
    _name_template = "QC {band} DISTORT RMS"
    _type = float
    _unit = "pixels"
    _description_template = "RMS of deviation of the distortion fit from expected values"


class QcDistortNSource(QcParameter):
    _name_template = "QC {band} DISTORT NSOURCE"
    _type = int
    _unit = "1"
    _description_template = "Number of positions used to fit the distortion polynomial"


class QcLmDistortRms(BandLmMixin, QcDistortRms):
    pass


class QcNDistortRms(BandNMixin, QcDistortRms):
    pass


class QcLmDistortNSource(BandLmMixin, QcDistortNSource):
    pass


class QcNDistortNSource(BandNMixin, QcDistortNSource):
    pass