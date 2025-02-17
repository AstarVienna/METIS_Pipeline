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
from typing import Dict

from pymetis.base import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs.common import SinglePipelineInput, MultiplePipelineInput, \
                            BadpixMapInput, MasterDarkInput, LinearityInput, \
                            RawInput, GainMapInput, PersistenceMapInput, \
                            WavecalInput, DistortionTableInput
from pymetis.inputs.mixins import PersistenceInputSetMixin
from pymetis.prefab.darkimage import DarkImageProcessor





class MetisIfuRsrfImpl(DarkImageProcessor):
    class InputSet(PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_RSRF_RAW")
            _title: str = "IFU rsrf raw"

        MasterDarkInput = MasterDarkInput

        class RsrfWcuOffInput(RawInput):
            """
            WCU_OFF input illuminated by the WCU up-to and including the
            integrating sphere, but no source.
            """
            _tags: re.Pattern = re.compile(r"IFU_WCU_OFF_RAW")
            _title: str = "IFU WCU off"

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.background = self.RsrfWcuOffInput(frameset)
            self.linearity = LinearityInput(frameset)
            self.gain_map = GainMapInput(frameset)
            self.distortion_table = DistortionTableInput(frameset)
            self.wavecal = WavecalInput(frameset)
            self.badpixmap = BadpixMapInput(frameset, required=False)

            self.inputs |= {self.background, self.linearity,
                            self.gain_map, self.distortion_table,
                            self.wavecal, self.badpixmap}

    class ProductBackground(PipelineProduct):
        """
        Intermediate product: the instrumental background (WCU OFF)
        """
        _tag = r"IFU_RSRF_BACKGROUND"
        _group = cpl.ui.Frame.FrameGroup.PRODUCT # TBC
        _level = cpl.ui.Frame.FrameLevel.INTERMEDIATE
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        # SKEL: copy product keywords from header
        def add_properties(self) -> None:
            super().add_properties()
            self.properties.append(self.header)


    class ProductMasterFlatIfu(PipelineProduct):
        _tag = r"MASTER_FLAT_IFU"
        _group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)


    class ProductRsrfIfu(PipelineProduct):
        _tag = r"RSRF_IFU"
        _group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE # set of 1D spectra?

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    class ProductBadpixMapIfu(PipelineProduct):
        _tag = r"BADPIX_MAP_IFU"
        _group = cpl.ui.Frame.FrameGroup.CALIB # TBC
        _level = cpl.ui.Frame.FrameLevel.FINAL
        _frame_type = cpl.ui.Frame.FrameType.IMAGE

        # SKEL: copy product keywords from header
        def add_properties(self):
            super().add_properties()
            self.properties.append(self.header)

    def process_images(self) -> [PipelineProduct]:
        # TODO: FUNC: basic raw processing of RSRF and WCU_OFF input frames:
        # - dark subtraction? (subtracting WCU_OFF frame might suffice?)
        # - gain / linearity correction? (as for dark subtraction)
        # - master dark will be used for bad-pixel map as a minimum

        # create bad pixel map
        # TODO: FUNC: create updated bad pixel map
        badpix_hdr = cpl.core.PropertyList()
        # placeholder data for now - bad-pixel map based on master_dark
        badpix_img = cpl.core.Image.load(self.inputset.master_dark.frame.file,
                                         extension=0)
        # TODO: create QC1 parameters:
        qc_badpix_count = 0
        # SKEL: Add QC keywords
        badpix_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                )
            )

        # create master WCU_OFF background image
        background_hdr = \
            cpl.core.PropertyList()
        # self.inputset.background.frameset.dump() # debug
        bg_images = self.load_images(self.inputset.background.frameset)
        background_img = self.combine_images(bg_images, "median") # if >2 images
        # TODO: SKEL: define usedframes?
        # TODO: SKEL: Add product keywords - currently none defined in DRLD

        # create 2D flat images (one for each raw input image?)
        spec_flat_hdr = \
            cpl.core.PropertyList()
        raw_images = self.load_images(self.inputset.raw.frameset)
        raw_images.subtract_image(background_img)
        # TODO: FUNC: group RSRF input frames (using EDPS wokflow?) by
        #   1. BB temperature
        #   2. int_sphere entrance aperture size
        #   3. Chopper mirror position
        # TODO: FUNC: collapse each group into a 2D image
        # TODO: FUNC: apply distortion correction and wavelength calibration
        # TODO: FUNC: divide each collapsed image by ideal continuum spec image,
        #   given T_BB_lamp and normalise
        # SKEL: Add QC keywords
        spec_flat_hdr.append(
            cpl.core.Property(
                "QC IFU RSRF NBADPIX",
                cpl.core.Type.INT,
                qc_badpix_count,
                )
            )

        # SKEL: placeholder single-file, single-extension data product for now
        spec_flat_img = self.combine_images(raw_images, "add")

        # create 1D RSRF
        # TODO: FUNC: average 2D flat in spatial direction for each trace
        rsrf_hdr = cpl.core.PropertyList()
        # TODO: SKEL: Add product keywords - currently none defined in DRLD
        # SKEL: placeholder data for now
        # NOTE: rebin() cpl documentation is incorrect -
        # ystart, xstart parameters are *1-based*, NOT 0-based
        img_height = spec_flat_img.height
        rsrf_img = spec_flat_img.rebin(1, 1, img_height, 1)
        rsrf_img.divide_scalar(img_height)

        product_background = self.ProductBackground(self, background_hdr, background_img)
        product_master_flat_ifu = self.ProductMasterFlatIfu(self, spec_flat_hdr, spec_flat_img)
        product_rsrf_ifu = self.ProductRsrfIfu(self, rsrf_hdr, rsrf_img)
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


class MetisIfuRsrf(MetisRecipe):
    _name = "metis_ifu_rsrf"
    _version = "0.1"
    _author = "Janus Brink"
    _email = "janus.brink27@gmail.com"
    _synopsis = "Determine the relative spectral response function."
    _description = """\
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

    Algorithm            
        Average / median stack WCU_OFF images to create background image
        Subtract background image from individual RSRF RAW frames
        TBC: subtract master_dark from above frames first?
        TBC: apply gain / linearity corrections to above frames?
        TBC: obtain bad pixel map from master_dark?
        Create continuum image by mapping Planck spectrum at Tlamp to wavelength
            image.
        Divide exposures by continuum image.
        Create master flat (2D RSRF) - TBC one extension per input exposure?
        Average in spatial direction to obtain relative response function
            (1D RSRF) - TBC multiple FITS extensions with spectral traces?
    """

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_ifu_rsrf.stacking.method",
            context="metis_ifu_rsrf",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])     # no parameters defined in DRLD

    implementation_class = MetisIfuRsrfImpl
