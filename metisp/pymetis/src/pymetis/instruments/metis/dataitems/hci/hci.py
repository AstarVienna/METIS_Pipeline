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

from pymetis.engine.dataitems import ImageDataItem
from pymetis.instruments.metis.mixins import CgrphRavcMixin, CgrphCvcMixin, CgrphAppMixin, BandLmMixin, BandNMixin


class OffAxisPsfRaw(ImageDataItem, abstract=True):
    _name_template = r'{band}_OFF_AXIS_PSF_RAW'
    _title_template = r"{band} off-axis PSF raw"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.RAW
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class AdiCalibrated(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_CALIBRATED'
    _title_template = r"{band} {cgrph} sci calibration"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class OnAxisPsfTemplate(ImageDataItem, abstract=True):
    _name_template = r'{band}_ON_AXIS_PSF_TEMPLATE'
    _title_template = r"{band} on-axis PSF template"
    _description_template = "calibration ADI image data" 
    _frame_group = cpl.ui.Frame.FrameGroup.CALIB
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }



class SciCentred(ImageDataItem, abstract=True):

    _name_template = r"{band}_{cgrph}_SCI_CENTRED"
    _title_template = r"{band} {cgrph} sci centred"
    _description_template = ""
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class CentroidTab(ImageDataItem, abstract=True):

    _name_template = r'{band}_{cgrph}_CENTROID_TAB'
    _title_template = r"{band} {cgrph} centroid tab"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }

class SciSpeckle(ImageDataItem, abstract=True):

    _name_template = r'{band}_{cgrph}_SCI_SPECKLE'
    _title_template = r"{band} {cgrph} sci speckle"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciHifilt(ImageDataItem, abstract=True):

    _name_template = r'{band}_{cgrph}_SCI_HIFILT'
    _title_template = r"{band} {cgrph} sci hifilt"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }

class SciDerotatedPsfsub(ImageDataItem, abstract=True):

    _name_template = r'{band}_{cgrph}_SCI_DEROTATED_PSFSUB'
    _title_template = r"{band} {cgrph} sci derotated psfsub"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class SciDerotated(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_DEROTATED'
    _title_template = r"{band} {cgrph} sci derotated"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class SciContrastRadprof(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_CONTRAST_RADPROF'
    _title_template = r"{band} {cgrph} sci contrast radprof"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }


class SciContrastAdi(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_CONTRAST_ADI'
    _title_template = r"{band} {cgrph} sci contrast adi"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }


class SciThroughput(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_THROUGHPUT'
    _title_template = r"{band} {cgrph} sci throughput"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Table,
    }


class SciCoverage(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_COVERAGE'
    _title_template = r"{band} {cgrph} sci coverage"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class SciSnr(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_SCI_SNR'
    _title_template = r"{band} {cgrph} sci snr"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }


class PsfMedian(ImageDataItem, abstract=True):
    _name_template = r'{band}_{cgrph}_PSF_MEDIAN'
    _title_template = r"{band} {cgrph} sci psf median"
    _description_template = "" 
    _frame_group = cpl.ui.Frame.FrameGroup.PRODUCT
    _frame_level = cpl.ui.Frame.FrameLevel.FINAL
    _oca_keywords = frozenset({'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'})

    _schema = {
        'PRIMARY': None,
        'DET1.DATA': Image,
    }



# --- Concrete data items ------------------------------------------------------
# One leaf per DRLD-defined tag. RAVC and CVC exist for both imager bands
# (recipe `metis_img_adi_cgrph`); APP exists for LM only and has no HIFILT
# product (recipe `metis_lm_adi_app`). See DRLD, Recipes_ADI.

class LmOffAxisPsfRaw(BandLmMixin, OffAxisPsfRaw):
    pass

class NOffAxisPsfRaw(BandNMixin, OffAxisPsfRaw):
    pass

class LmOnAxisPsfTemplate(BandLmMixin, OnAxisPsfTemplate):
    pass

class NOnAxisPsfTemplate(BandNMixin, OnAxisPsfTemplate):
    pass


class LmRavcCalibrated(BandLmMixin, CgrphRavcMixin, AdiCalibrated):
    pass

class LmRavcSciCentred(BandLmMixin, CgrphRavcMixin, SciCentred):
    pass

class LmRavcCentroidTab(BandLmMixin, CgrphRavcMixin, CentroidTab):
    pass

class LmRavcSciSpeckle(BandLmMixin, CgrphRavcMixin, SciSpeckle):
    pass

class LmRavcSciHifilt(BandLmMixin, CgrphRavcMixin, SciHifilt):
    pass

class LmRavcSciDerotatedPsfsub(BandLmMixin, CgrphRavcMixin, SciDerotatedPsfsub):
    pass

class LmRavcSciDerotated(BandLmMixin, CgrphRavcMixin, SciDerotated):
    pass

class LmRavcSciContrastRadprof(BandLmMixin, CgrphRavcMixin, SciContrastRadprof):
    pass

class LmRavcSciContrastAdi(BandLmMixin, CgrphRavcMixin, SciContrastAdi):
    pass

class LmRavcSciThroughput(BandLmMixin, CgrphRavcMixin, SciThroughput):
    pass

class LmRavcSciCoverage(BandLmMixin, CgrphRavcMixin, SciCoverage):
    pass

class LmRavcSciSnr(BandLmMixin, CgrphRavcMixin, SciSnr):
    pass

class LmRavcPsfMedian(BandLmMixin, CgrphRavcMixin, PsfMedian):
    pass


class LmCvcCalibrated(BandLmMixin, CgrphCvcMixin, AdiCalibrated):
    pass

class LmCvcSciCentred(BandLmMixin, CgrphCvcMixin, SciCentred):
    pass

class LmCvcCentroidTab(BandLmMixin, CgrphCvcMixin, CentroidTab):
    pass

class LmCvcSciSpeckle(BandLmMixin, CgrphCvcMixin, SciSpeckle):
    pass

class LmCvcSciHifilt(BandLmMixin, CgrphCvcMixin, SciHifilt):
    pass

class LmCvcSciDerotatedPsfsub(BandLmMixin, CgrphCvcMixin, SciDerotatedPsfsub):
    pass

class LmCvcSciDerotated(BandLmMixin, CgrphCvcMixin, SciDerotated):
    pass

class LmCvcSciContrastRadprof(BandLmMixin, CgrphCvcMixin, SciContrastRadprof):
    pass

class LmCvcSciContrastAdi(BandLmMixin, CgrphCvcMixin, SciContrastAdi):
    pass

class LmCvcSciThroughput(BandLmMixin, CgrphCvcMixin, SciThroughput):
    pass

class LmCvcSciCoverage(BandLmMixin, CgrphCvcMixin, SciCoverage):
    pass

class LmCvcSciSnr(BandLmMixin, CgrphCvcMixin, SciSnr):
    pass

class LmCvcPsfMedian(BandLmMixin, CgrphCvcMixin, PsfMedian):
    pass


class NRavcCalibrated(BandNMixin, CgrphRavcMixin, AdiCalibrated):
    pass

class NRavcSciCentred(BandNMixin, CgrphRavcMixin, SciCentred):
    pass

class NRavcCentroidTab(BandNMixin, CgrphRavcMixin, CentroidTab):
    pass

class NRavcSciSpeckle(BandNMixin, CgrphRavcMixin, SciSpeckle):
    pass

class NRavcSciHifilt(BandNMixin, CgrphRavcMixin, SciHifilt):
    pass

class NRavcSciDerotatedPsfsub(BandNMixin, CgrphRavcMixin, SciDerotatedPsfsub):
    pass

class NRavcSciDerotated(BandNMixin, CgrphRavcMixin, SciDerotated):
    pass

class NRavcSciContrastRadprof(BandNMixin, CgrphRavcMixin, SciContrastRadprof):
    pass

class NRavcSciContrastAdi(BandNMixin, CgrphRavcMixin, SciContrastAdi):
    pass

class NRavcSciThroughput(BandNMixin, CgrphRavcMixin, SciThroughput):
    pass

class NRavcSciCoverage(BandNMixin, CgrphRavcMixin, SciCoverage):
    pass

class NRavcSciSnr(BandNMixin, CgrphRavcMixin, SciSnr):
    pass

class NRavcPsfMedian(BandNMixin, CgrphRavcMixin, PsfMedian):
    pass


class NCvcCalibrated(BandNMixin, CgrphCvcMixin, AdiCalibrated):
    pass

class NCvcSciCentred(BandNMixin, CgrphCvcMixin, SciCentred):
    pass

class NCvcCentroidTab(BandNMixin, CgrphCvcMixin, CentroidTab):
    pass

class NCvcSciSpeckle(BandNMixin, CgrphCvcMixin, SciSpeckle):
    pass

class NCvcSciHifilt(BandNMixin, CgrphCvcMixin, SciHifilt):
    pass

class NCvcSciDerotatedPsfsub(BandNMixin, CgrphCvcMixin, SciDerotatedPsfsub):
    pass

class NCvcSciDerotated(BandNMixin, CgrphCvcMixin, SciDerotated):
    pass

class NCvcSciContrastRadprof(BandNMixin, CgrphCvcMixin, SciContrastRadprof):
    pass

class NCvcSciContrastAdi(BandNMixin, CgrphCvcMixin, SciContrastAdi):
    pass

class NCvcSciThroughput(BandNMixin, CgrphCvcMixin, SciThroughput):
    pass

class NCvcSciCoverage(BandNMixin, CgrphCvcMixin, SciCoverage):
    pass

class NCvcSciSnr(BandNMixin, CgrphCvcMixin, SciSnr):
    pass

class NCvcPsfMedian(BandNMixin, CgrphCvcMixin, PsfMedian):
    pass


class LmAppCalibrated(BandLmMixin, CgrphAppMixin, AdiCalibrated):
    pass

class LmAppSciCentred(BandLmMixin, CgrphAppMixin, SciCentred):
    pass

class LmAppCentroidTab(BandLmMixin, CgrphAppMixin, CentroidTab):
    pass

class LmAppSciSpeckle(BandLmMixin, CgrphAppMixin, SciSpeckle):
    pass

class LmAppSciDerotatedPsfsub(BandLmMixin, CgrphAppMixin, SciDerotatedPsfsub):
    pass

class LmAppSciDerotated(BandLmMixin, CgrphAppMixin, SciDerotated):
    pass

class LmAppSciContrastRadprof(BandLmMixin, CgrphAppMixin, SciContrastRadprof):
    pass

class LmAppSciContrastAdi(BandLmMixin, CgrphAppMixin, SciContrastAdi):
    pass

class LmAppSciThroughput(BandLmMixin, CgrphAppMixin, SciThroughput):
    pass

class LmAppSciCoverage(BandLmMixin, CgrphAppMixin, SciCoverage):
    pass

class LmAppSciSnr(BandLmMixin, CgrphAppMixin, SciSnr):
    pass

class LmAppPsfMedian(BandLmMixin, CgrphAppMixin, PsfMedian):
    pass
