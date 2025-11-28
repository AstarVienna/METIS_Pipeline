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
from pymetis.classes.mixins.cgrph import CgrphSpecificMixin


class IfuOffAxisPsfRaw(ImageDataItem, abstract=True):

    _name_template = r'IFU_OFF_AXIS_PSF_RAW'
    _title_template = r"IFU RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuAdiCalibrated(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_CALIBRATED'
    _title_template = r"IFU RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuOnAxisPsfTemplate(ImageDataItem, abstract=True):

    _name_template = r'IFU_ON_AXIS_PSF_TEMPLATE'
    _title_template = r"IFU RAVC sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }



class IfuSciCentred(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r"IFU_RAVC_SCI_CENTRED"
    _title_template = r"IFU RAVC sci centred"
    _description_template = ""
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuCentroidTab(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_CENTROID_TAB'
    _title_template = r"IFU RAVC centroid tab"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class IfuSciSpeckle(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_SPECKLE'
    _title_template = r"IFU RAVC sci speckle"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuSciHifilt(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_HIFILT'
    _title_template = r"IFU RAVC sci hifilt"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuSciDerotatedPsfsub(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_DEROTATED_PSFSUB'
    _title_template = r"IFU RAVC sci derotated psfsub"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuSciDerotated(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_DEROTATED'
    _title_template = r"IFU RAVC sci derotated"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuSciContrastRadprof(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_CONTRAST_RADPROF'
    _title_template = r"IFU RAVC sci contrast radprof"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class IfuSciContrastAdi(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_CONTRAST_ADI'
    _title_template = r"IFU RAVC sci contrast adi"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class IfuSciThroughput(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_THROUGHPUT'
    _title_template = r"IFU RAVC sci throughput"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class IfuSciCoverage(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_COVERAGE'
    _title_template = r"IFU RAVC sci coverage"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class IfuSciSnr(CgrphSpecificMixin, ImageDataItem, abstract=True):

    _name_template = r'IFU_RAVC_SCI_SNR'
    _title_template = r"IFU RAVC sci snr"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

# IFU data types for metis_ifu_adi_cgrph

class IfuRavcSciThroughput(CgrphAppMixin, IfuSciThroughput):
    pass

class IfuCvcSciThroughput(CgrphAppMixin, IfuSciThroughput):
    pass

class IfuAppSciThroughput(CgrphAppMixin, IfuSciThroughput):
    pass


class IfuAppCalibrated(CgrphAppMixin, IfuAdiCalibrated):
    pass

class IfuRavcCalibrated(CgrphRavcMixin, IfuAdiCalibrated):
    pass

class IfuCvcCalibrated(CgrphCvcMixin, IfuAdiCalibrated):
    pass


class IfuRavcSciCentred(CgrphAppMixin, IfuSciCentred):
    pass

class IfuRavcCentroidTab(CgrphAppMixin, IfuCentroidTab):
    pass

class IfuCvcSciCentred(CgrphAppMixin, IfuSciCentred):
    pass

class IfuCvcCentroidTab(CgrphAppMixin, IfuCentroidTab):
    pass

class IfuAppSciCentred(CgrphAppMixin, IfuSciCentred):
    pass

class IfuAppCentroidTab(CgrphAppMixin, IfuCentroidTab):
    pass


class IfuCvcSciSpeckle(CgrphAppMixin, IfuSciSpeckle):
    pass


class IfuCvcSciHifilt(CgrphAppMixin, IfuSciHifilt):
    pass

class IfuRavcSciSpeckle(CgrphAppMixin, IfuSciSpeckle):
    pass


class IfuRavcSciHifilt(CgrphAppMixin, IfuSciHifilt):
    pass

class IfuAppSciSpeckle(CgrphAppMixin, IfuSciSpeckle):
    pass


class IfuAppSciHifilt(CgrphAppMixin, IfuSciHifilt):
    pass


class IfuCvcSciDerotatedPsfsub(CgrphAppMixin, IfuSciDerotatedPsfsub):
    pass


class IfuCvcSciDerotated(CgrphAppMixin, IfuSciDerotated):
    pass



class IfuRavcSciDerotatedPsfsub(CgrphAppMixin, IfuSciDerotatedPsfsub):
    pass


class IfuRavcSciDerotated(CgrphAppMixin, IfuSciDerotated):
    pass



class IfuAppSciDerotatedPsfsub(CgrphAppMixin, IfuSciDerotatedPsfsub):
    pass


class IfuAppSciDerotated(CgrphAppMixin, IfuSciDerotated):
    pass


class IfuCvcSciContrastRadprof(CgrphAppMixin, IfuSciContrastRadprof):
    pass


class IfuCvcSciContrastAdi(CgrphAppMixin, IfuSciContrastAdi):
    pass


class IfuRavcSciContrastRadprof(CgrphAppMixin, IfuSciContrastRadprof):
    pass


class IfuRavcSciContrastAdi(CgrphAppMixin, IfuSciContrastAdi):
    pass


class IfuAppSciContrastRadprof(CgrphAppMixin, IfuSciContrastRadprof):
    pass


class IfuAppSciContrastAdi(CgrphAppMixin, IfuSciContrastAdi):
    pass


class IfuCvcSciCoverage(CgrphAppMixin, IfuSciCoverage):
    pass


class IfuCvcSciSnr(CgrphAppMixin, IfuSciSnr):
    pass


class IfuRavcSciCoverage(CgrphAppMixin, IfuSciCoverage):
    pass


class IfuRavcSciSnr(CgrphAppMixin, IfuSciSnr):
    pass


class IfuAppSciCoverage(CgrphAppMixin, IfuSciCoverage):
    pass


class IfuAppSciSnr(CgrphAppMixin, IfuSciSnr):
    pass
