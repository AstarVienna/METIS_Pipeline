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
from pymetis.dataitems.distortion import LmDistortionTable
from pymetis.dataitems.img.basicreduced import LmSciCalibrated
#from pymetis.dataitems.hci import LmOffAxisPsfRaw, LmOnAxisPsfTemplate
from pymetis.dataitems.hci.hci import LmAppCalibrated
from pymetis.dataitems.hci.hci import AdiCalibrated


from pymetis.dataitems.hci.hci import AdiCalibrated, LmAppSciCentred, LmAppCentroidTab
from pymetis.dataitems.hci.hci import LmAppSciSpeckle, LmAppSciHifilt, LmAppSciDerotatedPsfsub
from pymetis.dataitems.hci.hci import LmAppSciDerotated
from pymetis.dataitems.hci.hci import LmAppSciContrastRadprof, LmAppSciContrastAdi, LmAppSciThroughput
from pymetis.dataitems.hci.hci import LmAppSciCoverage, LmAppSciSnr, LmAppPsfMedian
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import RawInput
from pymetis.utils.dummy import create_dummy_header, create_dummy_image, create_dummy_table


class MetisLmAppSciCalibrateImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LmSciCalibrated
        #class LmOffAxisPsfRaw(RawInput):
        #    Item = OffAxisPsf
        #class LmOnAxisPsfTemplate(RawInput):
        #    Item = OnAxisPsfTemplate

        

    ProductLmSciCalibrated = LmAppCalibrated
    ProductLmSciCentred = LmAppSciCentred
    ProductLmCentroidTab = LmAppCentroidTab
    ProductLmSciSpeckle = LmAppSciSpeckle
    ProductLmSciHifilt = LmAppSciHifilt
    ProductLmSciDerotatedPsfsub = LmAppSciDerotatedPsfsub
    ProductLmSciDerotated = LmAppSciDerotated
    ProductLmSciContrastRadprof = LmAppSciContrastRadprof
    ProductLmSciContrastAdi = LmAppSciContrastAdi
    ProductLmSciThroughput = LmAppSciThroughput
    ProductLmSciCoverage = LmAppSciCoverage
    ProductLmSciSnr = LmAppSciSnr
    ProductLmSciPsfMedian = LmAppPsfMedian
    
    def process(self) -> set[DataItem]:
        
            image = self.inputset.raw.load_data('DET1.DATA')[0]
            #image = create_dummy_image()
            table = create_dummy_table()

            primary_header = create_dummy_header()
            header_lmSciCalibrated = create_dummy_header()
            header_lmSciCentred = create_dummy_header()
            header_lmCentroidTable = create_dummy_header()
            header_lmSciSpeckle = create_dummy_header()
            header_lmSciHifilt = create_dummy_header()
            header_lmSciDerotatedPsfsub = create_dummy_header()
            header_lmSciDerotated = create_dummy_header()
            header_lmSciContrastRadprof = create_dummy_header()
            header_lmSciContrastAdi = create_dummy_header()
            header_lmSciThroughput = create_dummy_header()
            header_lmSciCoverage = create_dummy_header()
            header_lmSciSnr = create_dummy_header()
            header_lmSciPsfMedian = create_dummy_header()
        
            
            product_lmSciCalibrated = self.ProductLmSciCalibrated(
                    primary_header,
                    Hdu(header_lmSciCalibrated, image, name='DET1.DATA'),
            )
            product_lmSciCentred = self.ProductLmSciCentred(
                    primary_header,
                    Hdu(header_lmSciCentred, image, name='DET1.DATA'),
            )
            product_lmCentroidTable = self.ProductLmCentroidTab(
                    primary_header,
                    Hdu(header_lmCentroidTable, table, name='DET1.DATA'),
            )
            product_lmSciSpeckle = self.ProductLmSciSpeckle(
                    primary_header,
                    Hdu(header_lmSciSpeckle, image, name='DET1.DATA'),
            )
            product_lmSciHifilt = self.ProductLmSciHifilt(
                    primary_header,
                    Hdu(header_lmSciHifilt, image, name='DET1.DATA'),
            )
            product_lmSciDerotatedPsfsub = self.ProductLmSciDerotatedPsfsub(
                    primary_header,
                    Hdu(header_lmSciDerotatedPsfsub, image, name='DET1.DATA'),
            )
            product_lmSciDerotated = self.ProductLmSciDerotated(
                    primary_header,
                    Hdu(header_lmSciDerotated, image, name='DET1.DATA'),
            )
            product_lmSciContrastRadprof = self.ProductLmSciContrastRadprof(
                    primary_header,
                    Hdu(header_lmSciContrastRadprof, table, name='DET1.DATA'),
            )
            product_lmSciContrastAdi = self.ProductLmSciContrastAdi(
                    primary_header,
                    Hdu(header_lmSciContrastAdi, table, name='DET1.DATA'),
            )
            product_lmSciThroughput = self.ProductLmSciThroughput(
                    primary_header,
                    Hdu(header_lmSciThroughput, table, name='DET1.DATA'),
            )
            product_lmSciCoverage = self.ProductLmSciCoverage(
                    primary_header,
                    Hdu(header_lmSciCoverage, image, name='DET1.DATA'),
            )
            product_lmSciSnr = self.ProductLmSciSnr(
                    primary_header,
                    Hdu(header_lmSciSnr, image, name='DET1.DATA'),
            )
            product_lmSciPsfMedian = self.ProductLmSciPsfMedian(
                    primary_header,
                    Hdu(header_lmSciPsfMedian, image, name='DET1.DATA'),
            )


        
            return {product_lmSciCalibrated, product_lmSciCentred, product_lmSciCentred, product_lmCentroidTable, product_lmSciHifilt, product_lmSciDerotatedPsfsub, product_lmSciDerotated, product_lmSciContrastRadprof, product_lmSciContrastAdi, product_lmSciThroughput, product_lmSciCoverage, product_lmSciSnr, product_lmSciPsfMedian}


class MetisLmAppSciCalibrated(MetisRecipe):
    _name: str = "metis_img_adi_cgrph"
    _version: str = "0.1"
    _author: str = "Jennifer Karr, A*"
    _email: str = "jkarr@asiaa.sinica.edu.tw"
    _synopsis: str = "ADI postprocssing"

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm = """TODO"""

    parameters = ParameterList([])

    Impl = MetisLmAppSciCalibrateImpl

    
