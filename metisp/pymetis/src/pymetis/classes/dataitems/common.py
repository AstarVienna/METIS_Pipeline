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
from abc import ABC

import cpl

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins import DetectorIfuMixin


class Raw(DataItem, ABC):
    _title: str = "raw frame"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW
    _description: str = "Abstract base class for all raw inputs. Please subclass."


class PersistenceMap(DataItem):
    _title: str = "persistence map"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Persistence map"


class LinearityMap(DataItem):
    _title: str = "linearity map"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Coefficients for the pixel non-linearity correction."
    _oca_keywords: set[str] = {'PRO.CATG'}


class FluxCalTable(DataItem):
    _title: str = "flux table"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Conversion between instrumental and physical flux units"
    _oca_keywords: set[str] = {'PRO.CATG'}


class PinholeTable(DataItem):
    _title: str = "pinhole table"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Table of pinhole locations"
    _oca_keywords: set[str] = {'PRO.CATG'}
    _pro_catg: str = r'PINHOLE_TABLE'


class IfuSciReduced(DetectorIfuMixin, DataItem):
    _title: str = "IFU science reduced"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.PRODUCT
    _description: str = "Reduced 2D detector image of science object."
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}


class TelluricCorrection(DataItem):
    _title: str = "telluric correction"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Telluric absorption correction."
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}


class AtmProfile(DataItem):
    _title: str = "atmosphere profile"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = ("Atmospheric profile containing height information on temperature, "
                         "pressure and molecular abundances")


class LsfKernel(DataItem):
    _title: str = "line spread function kernel"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Wavelength dependent model of the LSF"


class FluxStdCatalog(DataItem):
    _title: str = "catalog of standard stars"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Catalog of standard stars"


class SciCubeCalibrated(DataItem):
    _title: str = "rectified spectral cube"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."


class IfuSciCoadd(DetectorIfuMixin, DataItem):
    _title: str = "IFU science coadd"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = ("Spectral cube of science object, a coadd of a number of reduced IFU exposures"
                         "covering a different spatial and wavelength ranges.")
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}


class Rsrf(DataItem):
    _title: str = "RSRF"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "2D relative spectral response function"
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}


class IfuWavecal(DetectorIfuMixin, DataItem):
    _title: str = "wavelength calibration"
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
    _description: str = "Image with wavelength at each pixel"
    _oca_keywords: set[str] = {'PRO.CATG', 'DRS.IFU'}


class Combined(DataItem):
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _title: str = "spectral cube of science object"
    _description: str = "Spectral cube of standard star, combining multiple exposures."

