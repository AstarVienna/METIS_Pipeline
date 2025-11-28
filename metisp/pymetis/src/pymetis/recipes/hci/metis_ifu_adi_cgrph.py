"""
Thi file is part of the METIS Pipeline.
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

from pyesorex.parameter import ParameterList, ParameterEnum

# import the dataitems we use
from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.dataitems.distortion import IfuDistortionTable
from pymetis.dataitems.ifu import IfuSciReducedCube
#from pymetis.dataitems.hci import LmOffAxisPsfRaw, LmOnAxisPsfTemplate
from pymetis.dataitems.hci.ifuHci import IfuRavcCalibrated, IfuCvcCalibrated, IfuAppCalibrated


from pymetis.dataitems.hci.ifuHci import IfuRavcSciCentred, IfuRavcCentroidTab
from pymetis.dataitems.hci.ifuHci import IfuRavcSciSpeckle, IfuRavcSciHifilt, IfuRavcSciDerotatedPsfsub
from pymetis.dataitems.hci.ifuHci import IfuRavcSciDerotated
from pymetis.dataitems.hci.ifuHci import IfuRavcSciContrastRadprof, IfuRavcSciContrastAdi, IfuRavcSciThroughput
from pymetis.dataitems.hci.ifuHci import IfuRavcSciCoverage, IfuRavcSciSnr

from pymetis.dataitems.hci.ifuHci import IfuCvcSciCentred, IfuCvcCentroidTab
from pymetis.dataitems.hci.ifuHci import IfuCvcSciSpeckle, IfuCvcSciHifilt, IfuCvcSciDerotatedPsfsub
from pymetis.dataitems.hci.ifuHci import IfuCvcSciDerotated
from pymetis.dataitems.hci.ifuHci import IfuCvcSciContrastRadprof, IfuCvcSciContrastAdi, IfuCvcSciThroughput
from pymetis.dataitems.hci.ifuHci import IfuCvcSciCoverage, IfuCvcSciSnr

from pymetis.dataitems.hci.ifuHci import IfuAppSciCentred, IfuAppCentroidTab
from pymetis.dataitems.hci.ifuHci import IfuAppSciSpeckle, IfuAppSciHifilt, IfuAppSciDerotatedPsfsub
from pymetis.dataitems.hci.ifuHci import IfuAppSciDerotated
from pymetis.dataitems.hci.ifuHci import IfuAppSciContrastRadprof, IfuAppSciContrastAdi, IfuAppSciThroughput
from pymetis.dataitems.hci.ifuHci import IfuAppSciCoverage, IfuAppSciSnr
from pymetis.dataitems.ifu.ifu import IfuScienceCubeCalibrated

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import RawInput
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table



class MetisIfuRavcSciCalibrateImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuScienceCubeCalibrated
        #class LmOffAxisPsfRaw(RawInput):
        #    Item = OffAxisPsf
        #class LmOnAxisPsfTemplate(RawInput):
        #    Item = OnAxisPsfTemplate

        

    ProductIfuSciCalibrated = IfuRavcCalibrated
    ProductIfuSciCentred = IfuRavcSciCentred
    ProductIfuCentroidTab = IfuRavcCentroidTab
    ProductIfuSciSpeckle = IfuRavcSciSpeckle
    ProductIfuSciHifilt = IfuRavcSciHifilt
    ProductIfuSciDerotatedPsfsub = IfuRavcSciDerotatedPsfsub
    ProductIfuSciDerotated = IfuRavcSciDerotated
    ProductIfuSciContrastRadprof = IfuRavcSciContrastRadprof
    ProductIfuSciContrastAdi = IfuRavcSciContrastAdi
    ProductIfuSciThroughput = IfuRavcSciThroughput
    ProductIfuSciCoverage = IfuRavcSciCoverage
    ProductIfuSciSnr = IfuRavcSciSnr
    
    def process(self) -> set[DataItem]:
        
            image = self.inputset.raw.load_data('IMAGE')[0]
            #image = create_dummy_image()
            table = create_dummy_table()

            primary_header = create_dummy_header()
            header_ifuSciCalibrated = create_dummy_header()
            header_ifuSciCentred = create_dummy_header()
            header_ifuCentroidTable = create_dummy_header()
            header_ifuSciSpeckle = create_dummy_header()
            header_ifuSciHifilt = create_dummy_header()
            header_ifuSciDerotatedPsfsub = create_dummy_header()
            header_ifuSciDerotated = create_dummy_header()
            header_ifuSciContrastRadprof = create_dummy_header()
            header_ifuSciContrastAdi = create_dummy_header()
            header_ifuSciThroughput = create_dummy_header()
            header_ifuSciCoverage = create_dummy_header()
            header_ifuSciSnr = create_dummy_header()
        
            
            product_ifuSciCalibrated = self.ProductIfuSciCalibrated(
                    primary_header,
                    Hdu(header_ifuSciCalibrated, image, name='DET1.DATA'),
            )
            product_ifuSciCentred = self.ProductIfuSciCentred(
                    primary_header,
                    Hdu(header_ifuSciCentred, image, name='DET1.DATA'),
            )
            product_ifuCentroidTable = self.ProductIfuCentroidTab(
                    primary_header,
                    Hdu(header_ifuCentroidTable, table, name='DET1.DATA'),
            )
            product_ifuSciSpeckle = self.ProductIfuSciSpeckle(
                    primary_header,
                    Hdu(header_ifuSciSpeckle, image, name='DET1.DATA'),
            )
            product_ifuSciHifilt = self.ProductIfuSciHifilt(
                    primary_header,
                    Hdu(header_ifuSciHifilt, image, name='DET1.DATA'),
            )
            product_ifuSciDerotatedPsfsub = self.ProductIfuSciDerotatedPsfsub(
                    primary_header,
                    Hdu(header_ifuSciDerotatedPsfsub, image, name='DET1.DATA'),
            )
            product_ifuSciDerotated = self.ProductIfuSciDerotated(
                    primary_header,
                    Hdu(header_ifuSciDerotated, image, name='DET1.DATA'),
            )
            product_ifuSciContrastRadprof = self.ProductIfuSciContrastRadprof(
                    primary_header,
                    Hdu(header_ifuSciContrastRadprof, table, name='DET1.DATA'),
            )
            product_ifuSciContrastAdi = self.ProductIfuSciContrastAdi(
                    primary_header,
                    Hdu(header_ifuSciContrastAdi, table, name='DET1.DATA'),
            )
            product_ifuSciThroughput = self.ProductIfuSciThroughput(
                    primary_header,
                    Hdu(header_ifuSciThroughput, table, name='DET1.DATA'),
            )
            product_ifuSciCoverage = self.ProductIfuSciCoverage(
                    primary_header,
                    Hdu(header_ifuSciCoverage, image, name='DET1.DATA'),
            )
            product_ifuSciSnr = self.ProductIfuSciSnr(
                    primary_header,
                    Hdu(header_ifuSciSnr, image, name='DET1.DATA'),
            )
        
            return {product_ifuSciCalibrated, product_ifuSciCentred, product_ifuSciCentred, product_ifuCentroidTable, product_ifuSciHifilt, product_ifuSciDerotatedPsfsub, product_ifuSciDerotated, product_ifuSciContrastRadprof, product_ifuSciContrastAdi, product_ifuSciThroughput, product_ifuSciCoverage, product_ifuSciSnr}


class MetisIfuRavcSciCalibrated(MetisRecipe):
    _name: str = "metis_ifu_adi_cgrph"
    _version: str = "0.1"
    _author: str = "Jennifer Karr, A*"
    _email: str = "jkarr@asiaa.sinica.edu.tw"
    _synopsis: str = "ADI postprocssing"

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm = """TODO"""

    parameters = ParameterList([])

    Impl = MetisIfuRavcSciCalibrateImpl
    
