"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can edistribute it and/or modify
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
from cpl.core import Image, Table

from pymetis.classes.dataitems import ImageDataItem
from pymetis.classes.mixins import CgrphRavcMixin, CgrphCvcMixin, CgrphAppMixin 
from pymetis.classes.mixins import BandLmMixin
from pymetis.classes.mixins.band import BandSpecificMixin
from pymetis.classes.mixins.cgrph import CgrphSpecificMixin


class OffAxisPsfRaw(BandSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_OFF_AXIS_PSF_RAW'
    _title_template = r"{band} RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class AdiCalibrated(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_CALIBRATED'
    _title_template = r"{band} RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class OnAxisPsfTemplate(BandSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_ON_AXIS_PSF_TEMPLATE'
    _title_template = r"{band} RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }



class SciCentred(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r"{band}_RAVC_SCI_CENTRED"
    _title_template = r"{band} RAVC sci centred"
    _description_template = ""
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class CentroidTab(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_CENTROID_TAB'
    _title_template = r"{band} RAVC centroid tab"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class SciSpeckle(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_SPECKLE'
    _title_template = r"{band} RAVC sci speckle"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciHifilt(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_HIFILT'
    _title_template = r"{band} RAVC sci hifilt"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciDerotatedPsfsub(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_DEROTATED_PSFSUB'
    _title_template = r"{band} RAVC sci derotated psfsub"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciDerotated(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_DEROTATED'
    _title_template = r"{band} RAVC sci derotated"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciContrastRadprof(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_CONTRAST_RADPROF'
    _title_template = r"{band} RAVC sci contrast radprof"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class SciContrastAdi(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_CONTRAST_ADI'
    _title_template = r"{band} RAVC sci contrast adi"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class SciThroughput(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_THROUGHPUT'
    _title_template = r"{band} RAVC sci throughput"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class SciCoverage(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_COVERAGE'
    _title_template = r"{band} RAVC sci coverage"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciSnr(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_SNR'
    _title_template = r"{band} RAVC sci snr"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }
class PsfMedian(BandSpecificMixin, CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'{band}_RAVC_SCI_PSF_MEDIAN'
    _title_template = r"{band} RAVC sci psf median"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class LmRavcCalibrated(BandLmMixin, CgrphRavcMixin, AdiCalibrated):
    pass

class LmCvcCalibrated(BandLmMixin, CgrphCvcMixin, AdiCalibrated):
    pass

class LmAppCalibrated(BandLmMixin, CgrphAppMixin, AdiCalibrated):
    pass


class LmOffAxisPsfRaw(BandLmMixin, OffAxisPsfRaw):
    pass


class NOffAxisPsfRaw(BandLmMixin, OffAxisPsfRaw):
    pass


class LmOnAxisPsfTemplate(BandLmMixin, OnAxisPsfTemplate):
    pass


class NOnAxisPsfTemplate(BandLmMixin, OnAxisPsfTemplate):
    pass



class LmRavcSciCentred(BandLmMixin, CgrphAppMixin, SciCentred):
    pass


class LmRavcCentroidTab(BandLmMixin, CgrphAppMixin,  CentroidTab):
    pass


class LmRavcSciSpeckle(BandLmMixin, CgrphAppMixin, SciSpeckle):
    pass


class LmRavcSciHifilt(BandLmMixin, CgrphAppMixin, SciHifilt):
    pass


class LmRavcSciDerotatedPsfsub(BandLmMixin, CgrphAppMixin, SciDerotatedPsfsub):
    pass


class LmRavcSciDerotated(BandLmMixin, CgrphAppMixin, SciDerotated):
    pass


class LmRavcSciContrastRadprof(BandLmMixin, CgrphAppMixin, SciContrastRadprof):
    pass


class LmRavcSciContrastAdi(BandLmMixin, CgrphAppMixin, SciContrastAdi):
    pass


class LmRavcSciThroughput(BandLmMixin, CgrphAppMixin, SciThroughput):
    pass


class LmRavcSciCoverage(BandLmMixin, CgrphAppMixin, SciCoverage):
    pass


class LmRavcSciSnr(BandLmMixin, CgrphAppMixin, SciSnr):
    pass


class LmRavcPsfMedian(BandLmMixin, CgrphAppMixin,  PsfMedian):
    pass



class NRavcSciCentred(BandLmMixin, CgrphAppMixin,  SciCentred):
    pass


class NRavcCentroidTab(BandLmMixin, CgrphAppMixin,  CentroidTab):
    pass


class NRavcSciSpeckle(BandLmMixin, CgrphAppMixin,  SciSpeckle):
    pass


class NRavcSciHifilt(BandLmMixin, CgrphAppMixin,  SciHifilt):
    pass


class NRavcSciDerotatedPsfsub(BandLmMixin, CgrphAppMixin,  SciDerotatedPsfsub):
    pass


class NRavcSciDerotated(BandLmMixin, CgrphAppMixin,  SciDerotated):
    pass


class NRavcSciContrastRadprof(BandLmMixin, CgrphAppMixin,  SciContrastRadprof):
    pass


class NRavcSciContrastAdi(BandLmMixin, CgrphAppMixin,  SciContrastAdi):
    pass


class NRavcSciThroughput(BandLmMixin, CgrphAppMixin,  SciThroughput):
    pass


class NRavcSciCoverage(BandLmMixin, CgrphAppMixin,  SciCoverage):
    pass


class NRavcSciSnr(BandLmMixin, CgrphAppMixin,  SciSnr):
    pass


class NRavcPsfMedian(BandLmMixin, CgrphAppMixin,  PsfMedian):
    pass



class LmCvcSciCentred(BandLmMixin, CgrphAppMixin,  SciCentred):
    pass


class LmCvcCentroidTab(BandLmMixin, CgrphAppMixin,  CentroidTab):
    pass


class LmCvcSciSpeckle(BandLmMixin, CgrphAppMixin,  SciSpeckle):
    pass


class LmCvcSciHifilt(BandLmMixin, CgrphAppMixin,  SciHifilt):
    pass


class LmCvcSciDerotatedPsfsub(BandLmMixin, CgrphAppMixin,  SciDerotatedPsfsub):
    pass


class LmCvcSciDerotated(BandLmMixin, CgrphAppMixin,  SciDerotated):
    pass


class LmCvcSciContrastRadprof(BandLmMixin, CgrphAppMixin,  SciContrastRadprof):
    pass


class LmCvcSciContrastAdi(BandLmMixin, CgrphAppMixin,  SciContrastAdi):
    pass


class LmCvcSciThroughput(BandLmMixin, CgrphAppMixin,  SciThroughput):
    pass


class LmCvcSciCoverage(BandLmMixin, CgrphAppMixin,  SciCoverage):
    pass


class LmCvcSciSnr(BandLmMixin, CgrphAppMixin,  SciSnr):
    pass


class LmCvcPsfMedian(BandLmMixin, CgrphAppMixin,  PsfMedian):
    pass



class NCvcSciCentred(BandLmMixin, CgrphAppMixin,  SciCentred):
    pass


class NCvcCentroidTab(BandLmMixin, CgrphAppMixin, CentroidTab):
    pass


class NCvcSciSpeckle(BandLmMixin, CgrphAppMixin,  SciSpeckle):
    pass


class NCvcSciHifilt(BandLmMixin, CgrphAppMixin,  SciHifilt):
    pass


class NCvcSciDerotatedPsfsub(BandLmMixin, CgrphAppMixin,  SciDerotatedPsfsub):
    pass


class NCvcSciDerotated(BandLmMixin, CgrphAppMixin,  SciDerotated):
    pass


class NCvcSciContrastRadprof(BandLmMixin, CgrphAppMixin,  SciContrastRadprof):
    pass


class NCvcSciContrastAdi(BandLmMixin, CgrphAppMixin,  SciContrastAdi):
    pass


class NCvcSciThroughput(BandLmMixin, CgrphAppMixin,  SciThroughput):
    pass


class NCvcSciCoverage(BandLmMixin, CgrphAppMixin,  SciCoverage):
    pass


class NCvcSciSnr(BandLmMixin, CgrphAppMixin,  SciSnr):
    pass


class NCvcPsfMedian(BandLmMixin, CgrphAppMixin, PsfMedian):
    pass
