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
from pymetis.classes.mixins.band import BandLmMixin, BandNMixin, BandIfuMixin


class PersistenceMap(DataItem):
    _title = "persistence map"
    _name = "PERSISTENCE_MAP"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Persistence map"
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PERSISTENCE_MAP'


class LinearityMap(DataItem):
    _title = "linearity map"
    _name = "LINEARITY_MAP"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Coefficients for the pixel non-linearity correction."
    _oca_keywords = {'PRO.CATG'}


class FluxCalTable(DataItem):
    _title = "flux table"
    _name = "FLUXCAL_TABLE"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Conversion between instrumental and physical flux units"
    _oca_keywords = {'PRO.CATG'}


class PinholeTable(DataItem):
    _title = "pinhole table"
    _name = r'PINHOLE_TABLE'
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _type = cpl.ui.Frame.FrameType.TABLE
    _description = "Table of pinhole locations"
    _oca_keywords = {'PRO.CATG'}
    _pro_catg = r'PINHOLE_TABLE'


class TelluricCorrection(DataItem):
    _title = "telluric correction"
    _name = r'IFU_TELLURIC'
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Telluric absorption correction."
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class AtmProfile(DataItem):
    _title = "atmosphere profile"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = ("Atmospheric profile containing height information on temperature, "
                         "pressure and molecular abundances")


class LsfKernel(DataItem):
    _title = "line spread function kernel"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Wavelength dependent model of the LSF"


class FluxStdCatalog(DataItem):
    _name = r'FLUXSTD_CATALOG'
    _title = "catalog of standard stars"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Catalog of standard stars"
    _oca_keywords = set()


class SciCubeCalibrated(DataItem):
    _title = "rectified spectral cube"
    _name = r'IFU_SCI_CUBE_CALIBRATED'
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}
    _description = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."


class IfuSciCoadd(DetectorIfuMixin, DataItem):
    _name = r'IFU_SCI_COADD'
    _title = "IFU science coadd"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = ("Spectral cube of science object, a coadd of a number of reduced IFU exposures"
                         "covering a different spatial and wavelength ranges.")
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class Rsrf(DataItem):
    _title = "RSRF"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "2D relative spectral response function"
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class IfuWavecal(DetectorIfuMixin, DataItem):
    _name = r'IFU_WAVECAL'
    _title = "wavelength calibration"
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _description = "Image with wavelength at each pixel"
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class Combined(DataItem):
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _title = "spectral cube of science object"
    _description = "Spectral cube of a standard star, combining multiple exposures."


class ScienceCalibrated(DataItem, ABC):
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    @classmethod
    def name(cls):
        return rf'{cls.band()}_SCI_CALIBRATED'

    @classmethod
    def title(cls):
        return f"{cls.band()} science calibrated"


class LmScienceCalibrated(BandLmMixin, ScienceCalibrated):
    _description = "LM band image with flux calibration, WC coordinate system and distortion information"


class NScienceCalibrated(BandNMixin, ScienceCalibrated):
    _description = "N band image with flux calibration and distortion information"


class IfuScienceCubeCalibrated(BandIfuMixin, DataItem):
    _name = r'IFU_SCI_CUBE_CALIBRATED'
    _group = cpl.ui.Frame.FrameGroup.CALIB
    _title = "IFU science cube calibrated"
    _description = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."
    _oca_keywords = {'PRO.CATG', 'DRS.IFU'}


class AtmLineCatalog(DataItem):
    _name = r'ATM_LINE_CAT'
    _oca_keywords = {'PRO.CATG'}