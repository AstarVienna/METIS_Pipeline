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

import re
import cpl
from cpl.core import np
from cpl.core import Msg
ma = np.ma

# is this legal?
from astropy.table import QTable

from pymetis.classes.mixins import DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import (BadpixMapInput, MasterDarkInput, RawInput, GainMapInput,
                                    WavecalInput, DistortionTableInput, SinglePipelineInput, LinearityInput, OptionalInputMixin)
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin
from pymetis.classes.products import PipelineProduct
from pymetis.classes.products import TableProduct
from pymetis.classes.products import ProductBadpixMapDet

EXT = 1 # TODO: update to read multi-extension files
EXTRACT_WIDTH = 10 # TODO: make this a parameter

class MetisIfuRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_RSRF_RAW")
            _title: str = "IFU rsrf raw"
            _description: str = "Raw flats taken with black-body calibration lamp."

        class MasterDarkInput(MasterDarkInput):
            _tags: re.Pattern = re.compile(r"MASTER_DARK_IFU")

        class GainMapInput(GainMapInput):
            _tags: re.Pattern = re.compile(r"GAIN_MAP_IFU")

        class LinearityInput(LinearityInput):
            _tags: re.Pattern = re.compile(r"LINEARITY_IFU")

        class RsrfWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"IFU_WCU_OFF_RAW")
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "IFU WCU off"
            _description: str = "Raw data for dark subtraction in other recipes."

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _tags: re.Pattern = re.compile(r"BADPIX_MAP_IFU")

        DistortionTableInput = DistortionTableInput
        WavecalInput = WavecalInput

    class ProductRsrfBackground(PipelineProduct):
        """
        Intermediate product: the instrumental background (WCU OFF)
        """
        _tag: str = r"IFU_RSRF_BACKGROUND"
        group = cpl.ui.Frame.FrameGroup.PRODUCT # TBC
        level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description: str = "something"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

        # SKEL: copy product keywords from header
        def add_properties(self) -> None:
            super().add_properties()
            self.properties.append(self.header)


    class ProductMasterFlatIfu(PipelineProduct):
        _tag: str = r"MASTER_FLAT_IFU"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        _description: str = "Master flat frame for IFU image data"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)


    class ProductRsrfIfu(TableProduct):
        _tag: str = r"RSRF_IFU"
        group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE

        _description: str = "1D relative spectral response function"
        _oca_keywords = {'PRO.CATG', 'DRS.IFU'}

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    class ProductBadpixMapIfu(DetectorIfuMixin, ProductBadpixMapDet):
        pass

    def process_images(self) -> [PipelineProduct]:
        # TODO: FUNC: basic raw processing of RSRF and WCU_OFF input frames:
        # - dark subtraction? (subtracting WCU_OFF frame should suffice?)
        # - gain / linearity correction?
        # - master dark will be used for bad-pixel map as a minimum

        # create bad pixel map
        # TODO: FUNC: create updated bad pixel map
        badpix_hdr = cpl.core.PropertyList()
        # placeholder data for now - bad-pixel map based on master_dark
        badpix_img = cpl.core.Image.load(self.inputset.master_dark.frame.file,
                                         extension=0)
        # TODO: create QC1 parameters:
        qc_badpix_count = 0
        # Add QC keywords
        badpix_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                )
            )

        # Msg.debug(self.__class__.__qualname__,
        #     f"Dist table file: {self.inputset.distortion_table.frame.file}")

        ## load IFU trace definition file - only one extension for now
        trace_list = self.read_ifu_distortion_table(self.inputset.distortion_table.frame.file, EXT)

        ## load wavelength calibration image
        wavecal_img = cpl.core.Image.load(
            self.inputset.wavecal.frame.file, extension=EXT)

        ## create master WCU_OFF background image
        background_hdr = \
            cpl.core.PropertyList()
        # self.inputset.background.frameset.dump() # debug
        bg_images = self.load_images(self.inputset.rsrf_wcu_off.frameset)
        background_img = self.combine_images(bg_images, "median") # if >2 images
        
        # TODO: define usedframes?
        # TODO: Add product keywords - currently none defined in DRLD

        ## create 2D flat image (raw images are added together)
        spec_flat_hdr = \
            cpl.core.PropertyList()
        
        # load RSRF_RAW images, stack them and subtract the background
        raw_images = self.load_images(self.inputset.raw.frameset)

        # obtain black-body temperature from first frame's header
        # NOTE: this assumes raw frames were grouped by BB temperature
        rsrf_raw_hdr = cpl.core.PropertyList.load(
            self.inputset.raw.frameset[0].file,
            position=0)
        bb_temp = rsrf_raw_hdr['WCU_BB_TEMP'].value

        # TODO: make stacking method a parameter
        # SKEL: placeholder single-file, single-extension data product for now
        spec_flat_img = self.combine_images(raw_images, "median") # if >2 images
        spec_flat_img.subtract(background_img)

        # SKEL: Add QC keywords
        spec_flat_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                )
            )

        ## create black-body image
        bb_img = self.create_ifu_blackbody_image(wavecal_img, bb_temp)

        # scale the BB image to the RSRF image before dividing
        raw_level = spec_flat_img.get_max()
        bb_level = bb_img.get_median()
        bb_img.multiply_scalar(raw_level / bb_level)

        # divide the RSRF image by the black-body spectrum
        # (inherits the bb bad-pixel mask)
        spec_flat_img.divide(bb_img)

        # extract 1D RSRF curves
        rsrf_1d_list = self.extract_ifu_1d_spectra(spec_flat_img, trace_list,
                                                   trace_width=EXTRACT_WIDTH)

        # global normalisation of the 1D RSRF curves
        rsrf_med = np.zeros(len(rsrf_1d_list))
        for i in range(len(rsrf_1d_list)):
            # avoid calling cpl.core.Vector.median() as this sorts the vector!
            rsrf_med[i] = np.median(np.array(rsrf_1d_list[i]))

        scale = np.mean(rsrf_med)

        # TODD: exception for zero scale
        for i in range(len(rsrf_1d_list)):
                rsrf_1d_list[i].divide_scalar(scale)

        # create 1D RSRF product
        rsrf_hdr = cpl.core.PropertyList()
        # TODO: FUNC: Add product keywords - currently none defined in DRLD
        from astropy import table
        table = table.QTable()
        for i, rsrf in enumerate(rsrf_1d_list, start=1):
            table[f'rsrf_{i}'] = rsrf
            table[f'rsrf_{i}'].name = f'rsrf_{i}'
            table[f'rsrf_{i}'].unit = 'normalised'
            table[f'rsrf_{i}'].description = '1D RSRF curve {i}'

        rsrf_table = cpl.core.Table(table)

        product_background = self.ProductRsrfBackground(self, background_hdr, background_img)
        product_master_flat_ifu = self.ProductMasterFlatIfu(self, spec_flat_hdr, spec_flat_img)
        product_rsrf_ifu = self.ProductRsrfIfu(self, rsrf_hdr, rsrf_table)
        product_badpix_map_ifu = self.ProductBadpixMapIfu(self, badpix_hdr, badpix_img)

        return [product_background, product_master_flat_ifu, product_rsrf_ifu, product_badpix_map_ifu]

    def load_images(self, frameset: cpl.ui.FrameSet) -> cpl.core.ImageList:
        """Load an imagelist from a FrameSet

        This is a temporary implementation that should be generalised to the
        entire pipeline package. It uses cpl functions - these should be
        replaced with hdrl functions once they become available, in order
        to use uncertainties and masks.
        """
        output = cpl.core.ImageList()

        for idx, frame in enumerate(frameset):
            cpl.core.Msg.info(self.__class__.__qualname__,
                     f"Processing input frame #{idx}: {frame.file!r}...")
            output.append(cpl.core.Image.load(frame.file, extension=1))

        return output

    def create_ifu_blackbody_image(self, wavecal_img, bb_temp):
        """
        Create a blackbody image from the RSRF image and the wavelength calibration image.
        """
        
        wdata = wavecal_img.as_array()

        # create a new image to hold the black-body spectrum
        # each pixel will hold the BB flux at the wavelength of that pixel
        bb_data = np.zeros_like(wdata)

        # create wavelength lookup table [im um]
        wlookup = np.unique(wdata)[1:]  # remove the first element (0)
        wavelengths = cpl.core.Vector(wlookup / 1e6)  # Wavelengths in meters

        # Calculate the black-body flux at each wavelength
        flux = cpl.drs.photom.fill_blackbody(cpl.drs.photom.Unit.LESS,  # output unit
                                    wavelengths,  # Wavelengths in meters
                                    cpl.drs.photom.Unit.LENGTH,  # input unit
                                    bb_temp  # Temperature in Kelvin
                                    )

        # convert lookup table to vector for binary search functionality
        wlookup = cpl.core.Vector(wlookup)

        # fill the BB data array with the flux values
        for i in range(bb_data.shape[0]):
            for j in range(bb_data.shape[1]):
                if wdata[i, j] > 0: # only fill valid pixels
                    # find the index of the closest wavelength in the lookup table
                    # and assign the corresponding flux value
                    bb_data[i, j] = flux[wlookup.binary_search(wdata[i, j])]

        # mask the zero values with the bad-pixel mask to avoid division by zero
        bb_img = cpl.core.Image(bb_data)
        bb_img.reject_value({0})

        return bb_img

    def read_ifu_distortion_table(self, fits_file, ext=1) -> list:
        """
        Read the IFU distortion table from the given FITS file.

        Parameters:
        fits_file (str): Path to the FITS file containing the distortion table.

        Returns:
        list: A list of tuples containing the x- and y-coordinates of the traces.
        """
        # Load the distortion table
        # TODO: assumes distortion table has one set of coefficients for each extension
        distortion_table = cpl.core.Table.load(fits_file, xtnum=ext)

        # obtain the trace polynomials from the distortion table
        trace_polys = distortion_table.column_array('orders')[0]
        x_ranges = distortion_table.column_array('column_range')[0]

        # create a list of y-coordinates for each trace from the distortion table
        # x_arr = np.arange(0, rsrf_raw_img.width)
        trace_list = []
        for x_range, trace in zip(x_ranges, trace_polys):
            x_arr = np.arange(x_range[0], x_range[1])
            poly_n = len(trace) - 1
            y_arr = [sum([k*x**(poly_n-i) for i, k in enumerate(trace)]) for x in x_arr]
            trace_list.append((x_arr, y_arr))
            
        # return the list of x,y coordinates for each trace
        return trace_list
        
    def extract_ifu_1d_spectra(self, img, trace_list, trace_width=10) -> list:
        """
        Extract 1D spectra from the given image using the provided list of
        spectral trace coordinates.

        Parameters:
        img : cpl.core.Image
            The image from which to extract the spectra.
        trace_list : list of tuples
            Each tuple contains two arrays: x-coordinates and y-coordinates
            of the spectral trace.
        trace_width : int
            The width of the trace to be used for extraction.
        
        Returns:
        list of cpl.core.Vector
            A list of 1D spectra extracted from the image.
        """
        
        # copy data to np.masked_array for processing
        rej_mask = np.array(img.bpm)  # set bad pixels
        mdata = ma.masked_array(img.as_array(), mask=rej_mask)
        imwidth = img.width

        # TODO: check that traces are within the image bounds

        # create a list of 1D RSRF curves (width is the image width)
        rsrf_1d_list = []
        for trace in trace_list:
            x_arr = trace[0]
            y_arr = trace[1]
            rsrf_1d = np.zeros(imwidth, dtype=float)
            for i, x in enumerate(x_arr):
                yc = y_arr[i]
                rsrf_1d[x] = mdata[int(yc-trace_width):int(yc+trace_width), x].mean()
            rsrf_1d_list.append(cpl.core.Vector(rsrf_1d))
        
        return rsrf_1d_list

class MetisIfuRsrf(MetisRecipe):
    _name: str = "metis_ifu_rsrf"
    _version: str = "0.1"
    _author: str = "Janus Brink, A*"
    _email: str = "janus.brink27@gmail.com"
    _synopsis: str = "Determine the relative spectral response function for the IFU detector."
    _undescription: str = """\
    Create relative spectral response function for the IFU detector

    Inputs
        IFU_RSRF_RAW:    Raw RSRF images [1-n]
        IFU_WCU_RAW_OFF: Background images with WCU black-body closed [1-n]
        MASTER_DARK_IFU: Master dark frame [optional?]
        BADPIX_MAP_IFU:  Bad-pixel map for 2RG detector [optional]
        PERSISTENCE_MAP: Persistence map [optional]
        GAIN_MAP_IFU:    Gain map for 2RG detector
        LINEARITY_IFU:   Linearity map for 2RG detector
        IFU_DISTORTION_TABLE: Distortion coefficients for an IFU data set
        IFU_WAVECAL:     IFU wavelength calibration

    Matched Keywords
        DET.DIT
        DET.NDIT
        DRS.IFU
    
    Outputs
        MASTER_FLAT_IFU: Master flat frame for IFU image data
        RSRF_IFU: 1D relative spectral response function
        BADPIX_MAP_IFU: Updated bad-pixel map
    """ # FixMe This is currently not shown anywhere

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """Average / median stack WCU_OFF images to create background image
        Subtract background image from individual RSRF RAW frames
        TBC: subtract master_dark from above frames first?
        TBC: apply gain / linearity corrections to above frames?
        TBC: obtain bad pixel map from master_dark?
        Create continuum image by mapping Planck spectrum at Tlamp to wavelength image.
        Divide exposures by continuum image.
        Create master flat (2D RSRF) - TBC one extension per input exposure?
        Average in spatial direction to obtain relative response function
            (1D RSRF) - TBC multiple FITS extensions with spectral traces?"""

    implementation_class = MetisIfuRsrfImpl
